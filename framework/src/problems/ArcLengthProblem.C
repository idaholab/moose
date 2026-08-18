//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArcLengthProblem.h"

#include "ArcLengthNonlinearSystem.h"
#include "Assembly.h"
#include "AuxiliarySystem.h"
#include "Conversion.h"
#include "MooseEnum.h"
#include "MooseUtils.h"
#include "MooseVariableBase.h"
#include "NodalBCBase.h"

#include "libmesh/int_range.h"
#include "libmesh/node.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/parallel.h"
#include "libmesh/petsc_nonlinear_solver.h"
#include "libmesh/petsc_vector.h"
#include "libmesh/sparse_matrix.h"

#include <cmath>
#include <limits>

using namespace libMesh;

registerMooseObject("MooseApp", ArcLengthProblem);

InputParameters
ArcLengthProblem::validParams()
{
  InputParameters params = FEProblemBase::validParams();
  params.addClassDescription("Problem that traces an equilibrium path with arc-length "
                             "continuation, scaling the residual objects routed to the load tag "
                             "by a load parameter that is an unknown of the solve.");

  params.addParam<TagName>("load_vector_tag",
                           "arc_length_load",
                           "Name of the vector tag that marks the residual objects making up the "
                           "load. Put this name in the replacing parameter 'vector_tags' of every "
                           "load object.");
  params.addParam<TagName>("load_matrix_tag",
                           "arc_length_load_jac",
                           "Name of the matrix tag that holds the derivative of the load with "
                           "respect to the solution. Put this name in the replacing parameter "
                           "'matrix_tags' of a deformation dependent, or follower, load object.");
  params.addRequiredRangeCheckedParam<Real>(
      "step_size",
      "step_size > 0",
      "Arc length increment taken per continuation step, at every time step size. A TimeStepper "
      "cutback shrinks the load span a transient step covers and leaves this turning radius "
      "alone, because the solution excursion across a sharp turn of the path does not shrink "
      "with the span.");
  params.addRangeCheckedParam<unsigned int>(
      "max_continuation_steps",
      100,
      "max_continuation_steps > 0",
      "Maximum number of continuation steps of a one-shot path. A path that spends the whole "
      "of this budget ends there and is reported as a converged solve, "
      "which is what the default does. Set "
      "'end_on_max_continuation_steps' to false to fail the solve there "
      "instead. A transient run owns this: every time step advances the trace by a single "
      "increment.");
  params.addParam<Real>("lambda_max",
                        1.0,
                        "Load parameter a one-shot continuation stops at. This is one of the two "
                        "endings a completed path has, the other being the step budget of "
                        "'max_continuation_steps', and it is the only one left when "
                        "'end_on_max_continuation_steps' is set to false. A transient run owns "
                        "this: end the run with the executioner, or with a Terminator watching "
                        "the load parameter postprocessor.");
  params.addParam<Real>("lambda_min",
                        -1.0,
                        "Lower clamp on the load parameter of a one-shot continuation. A "
                        "continuation step that would take the load parameter below this value is "
                        "truncated to it, which turns the path back the way it came rather than "
                        "ending it, so this is a bound on where the path may go and never an exit "
                        "criterion. A transient run owns this: a step loads or unloads at most "
                        "its own increment scale, and the trace descends across steps.");
  params.addRangeCheckedParam<Real>("psi_squared",
                                    1.0,
                                    "psi_squared >= 0",
                                    "Weight of the load parameter term in the arc-length "
                                    "constraint. Zero gives the cylindrical constraint, which "
                                    "measures the arc length with the solution alone.");
  params.addParam<MooseEnum>("correction_type",
                             MooseEnum("exact normal", "exact"),
                             "Scheme that corrects an iterate back onto the arc-length constraint "
                             "surface. 'exact' solves the quadratic constraint, 'normal' takes a "
                             "step orthogonal to the increment.");
  params.addParam<TagName>(
      "held_load_vector_tag",
      "Name of a vector tag that holds the constant loads a transient continuation is run under, "
      "such as a preload held while the tagged load is continued. Route each held load with "
      "'extra_vector_tags', which leaves it in the residual it already contributes to and fills "
      "this tag with a copy, and create the tag with 'extra_tag_vectors'. The dissipation "
      "judgement of a descending step then accounts for the work the held loads do: without it, "
      "unloading elastically under a held load reads as dissipative and a step that walks back "
      "down an elastic branch is accepted as a descent along the path.");
  params.addParam<bool>(
      "use_continuation",
      true,
      "Whether a transient step runs a continuation. A step solved with this false is an ordinary "
      "Newton solve with the whole of its load increment applied as a prescribed ramp along the "
      "current direction of travel, so the load factor equals the time and one solve covers one "
      "step. The parameter is controllable: a [Controls] object can switch the continuation on "
      "or off per step from a function, a postprocessor, or any other trigger, with the committed "
      "load factor carrying across every switch. This replaces the two-input pattern of ramping "
      "a load with plain solves, writing a checkpoint, and restarting the continuation from it.");
  params.declareControllable("use_continuation");
  params.addParam<bool>("end_on_max_continuation_steps",
                        true,
                        "Whether a one-shot continuation that spends the whole of "
                        "'max_continuation_steps' ends the path successfully instead of failing "
                        "the solve, which is what the default does. This is what traces a "
                        "softening branch, where the load parameter falls monotonically past the "
                        "peak and 'lambda_max' is never reachable, so the step budget is the end "
                        "of the path. Set this to false to fail the solve on a spent budget "
                        "instead, which leaves 'lambda_max' as the only ending a path has. The "
                        "exit is read off the count of increments a continuation traced, so it "
                        "needs a single solve per path: a setup that solves the same path more "
                        "than once carries that count past the budget and reports the solve as "
                        "failed. A transient run owns this: the internal budget of one increment "
                        "per step is always a designed ending there.");

  params.addParamNamesToGroup("load_vector_tag load_matrix_tag", "Tagging");
  params.addParamNamesToGroup("step_size max_continuation_steps "
                              "lambda_max lambda_min psi_squared correction_type "
                              "end_on_max_continuation_steps use_continuation",
                              "Arc length continuation");

  return params;
}

ArcLengthProblem::ArcLengthProblem(const InputParameters & parameters)
  : FEProblemBase(parameters),
#if PETSC_RELEASE_GREATER_EQUALS(3, 22, 0)
    _correction_type(correctionType(getParam<MooseEnum>("correction_type"))),
    _end_on_max_continuation_steps(getParam<bool>("end_on_max_continuation_steps")),
    _use_continuation(getParam<bool>("use_continuation")),
    _retrace_handled(false),
    _increment(0),
    _ended_on_spent_budget(false),
    _checked_load_on_constrained_dofs(false),
    _step_lambda(0.0),
    _nominal_dt(declareRestartableData<Real>("nominal_dt", 0.0)),
    _path_direction(declareRestartableData<int>("path_direction", 1)),
    _lambda_accum(declareRestartableData<Real>("lambda_accum", 0.0)),
    _step_load_increment(declareRestartableData<Real>("step_load_increment", 0.0)),
#endif
    _load_parameter(declareRestartableData<Real>("load_parameter", 0.0))
{
#if PETSC_RELEASE_GREATER_EQUALS(3, 22, 0)
  if (_nl_sys_names.size() != 1)
    paramError("nl_sys_names", "Arc-length continuation drives a single nonlinear system.");
  if (_linear_sys_names.size())
    paramError("linear_sys_names", "Arc-length continuation drives a single nonlinear system.");

  for (const auto i : index_range(_nl_sys_names))
  {
    auto & nl = _nl[i];
    nl = std::make_shared<ArcLengthNonlinearSystem>(*this,
                                                    _nl_sys_names[i],
                                                    getParam<TagName>("load_vector_tag"),
                                                    getParam<TagName>("load_matrix_tag"));
    _arclength_nl = std::dynamic_pointer_cast<ArcLengthNonlinearSystem>(nl);
    _solver_systems[i] = std::dynamic_pointer_cast<SolverSystem>(nl);
    nl->system().prefer_hash_table_matrix_assembly(_use_hash_table_matrix_assembly);
  }

  // backwards compatibility for AD for objects that depend on initializing derivatives during
  // construction
  setCurrentNonlinearSystem(0);

  _aux = std::make_shared<AuxiliarySystem>(*this, "aux0");

  newAssemblyArray(_solver_systems);

  initNullSpaceVectors(parameters, _nl);

  // Create extra vectors if any
  createTagVectors();

  // Create extra solution vectors if any
  createTagSolutions();

  // The load Jacobian is assembled along with the standard Jacobian, which only happens for a tag
  // the system holds a matrix for. Whether any follower load exists is not known until the
  // objects have been added, so the matrix is always allocated.
  _arclength_nl->addMatrix(_arclength_nl->loadMatrixTag());
#else
  mooseError("Arc-length continuation requires PETSc 3.22.0 or newer. It is done by PETSc's "
             "SNESNEWTONAL solver, which PETSc added in that release.");
#endif
}

#if PETSC_RELEASE_GREATER_EQUALS(3, 22, 0)

SNESNewtonALCorrectionType
ArcLengthProblem::correctionType(const MooseEnum & correction_type)
{
  if (correction_type == "exact")
    return SNES_NEWTONAL_CORRECTION_EXACT;
  else if (correction_type == "normal")
    return SNES_NEWTONAL_CORRECTION_NORMAL;
  else
    ::mooseError("Unknown 'correction_type' '", correction_type, "'.");
}

void
ArcLengthProblem::initialSetup()
{
  FEProblemBase::initialSetup();

  // SNESNEWTONAL takes its continuation settings from the options database only. The mapping is
  // done here, after the executioner has stored the options from the input, so that these win,
  // and MOOSE applies them along with the rest on every solve. A transient run owns the budget
  // and the clamps: every step advances the trace by a single increment, whose load parameter
  // spans the increment scale of that step alone, so the budget is one and the clamps are the
  // symmetric span: a step loads or unloads at most its own increment scale, and the corrector
  // is boxed against the runaway oscillation an open floor admits at a sharp turn of the path.
  // The step size stored here is rewritten by onTimestepBegin with the radius of every attempt.
  auto & pairs = getPetscOptions().pairs;
  pairs.emplace_back("-snes_newtonal_step_size",
                     Moose::stringifyExact(getParam<Real>("step_size")));
  pairs.emplace_back(
      "-snes_newtonal_max_continuation_steps",
      _transient ? "1" : Moose::stringify(getParam<unsigned int>("max_continuation_steps")));
  pairs.emplace_back("-snes_newtonal_psisq", Moose::stringifyExact(getParam<Real>("psi_squared")));
  pairs.emplace_back("-snes_newtonal_lambda_min",
                     _transient ? "-1" : Moose::stringifyExact(getParam<Real>("lambda_min")));
  pairs.emplace_back("-snes_newtonal_lambda_max",
                     _transient ? "1" : Moose::stringifyExact(getParam<Real>("lambda_max")));
  // The tangent load always comes from the callback below, never from a right hand side vector
  pairs.emplace_back("-snes_newtonal_scale_rhs", "false");
}

void
ArcLengthProblem::checkProblemIntegrity()
{
  FEProblemBase::checkProblemIntegrity();

  if (solverParams()._type != Moose::ST_NEWTON)
    mooseError("Arc-length continuation requires solve_type = NEWTON. It solves for the variation "
               "of the solution with respect to the load parameter with the assembled Jacobian, "
               "which a matrix free solve type never forms.");

  // MOOSE upper cases the option names it stores from the input, and PETSc reads them back case
  // insensitively, so the name has to be folded before it is compared
  for (const auto & option : getPetscOptions().pairs)
    if (MooseUtils::toLower(option.first) == "-snes_type")
      mooseError("-snes_type is owned by ArcLengthProblem and cannot be set in "
                 "petsc_options_iname. PETSc applies the options database after the problem has "
                 "set the arc-length solver type, so this option would silently replace it and "
                 "the solve would follow a prescribed load instead of the equilibrium path.");

  if (!_arclength_nl->hasLoadObjects())
    paramError("load_vector_tag",
               "Arc-length continuation requires at least one residual object routed to the load "
               "vector tag '",
               getParam<TagName>("load_vector_tag"),
               "'. Put vector_tags = '",
               getParam<TagName>("load_vector_tag"),
               "' on the residual object that carries the load.");

  // A transient run owns the budget and the clamps of its per-step increments, so a value from
  // the input would be silently ignored and is refused instead
  if (_transient)
    for (const auto & name :
         {"max_continuation_steps", "end_on_max_continuation_steps", "lambda_max", "lambda_min"})
      if (isParamSetByUser(name))
        paramError(name,
                   "'",
                   name,
                   "' belongs to a one-shot continuation alone. A transient run advances the "
                   "trace by a single increment per time step, so it owns the step budget and the "
                   "load parameter clamps: control the radius with 'step_size' and the "
                   "TimeStepper, and end the run with the executioner or a Terminator watching "
                   "the load parameter postprocessor.");
}

PetscErrorCode
ArcLengthProblem::arcLengthTangentLoad(SNES /*snes*/, Vec x, Vec q, void * context)
{
  PetscFunctionBegin;

  static_cast<ArcLengthProblem *>(context)->computeTangentLoad(x, q);

  PetscFunctionReturn(PETSC_SUCCESS);
}

PetscErrorCode
ArcLengthProblem::arcLengthUpdate(SNES snes, PetscInt /*step*/)
{
  PetscFunctionBegin;

  void * context;
  LibmeshPetscCallQ(SNESGetApplicationContext(snes, &context));
  static_cast<ArcLengthProblem *>(context)->executeArcLengthIncrement(snes);

  PetscFunctionReturn(PETSC_SUCCESS);
}

bool
ArcLengthProblem::inContinuation() const
{
  // 'use_continuation' is controllable and read through a reference, so a [Controls] object that
  // flips it is obeyed by the next solve. A steady solve is one whole continuation and has no
  // prescribed phase.
  return !_transient || _use_continuation;
}

void
ArcLengthProblem::initPetscOutputAndSomeSolverSettings()
{
  // Creates the SNES and puts the MOOSE defaults, including the convergence test replaced below,
  // on it
  FEProblemBase::initPetscOutputAndSomeSolverSettings();

  // A prescribed step keeps the solver MOOSE set up: an ordinary Newton solve, judged by the
  // ordinary convergence test, at the load factor updateLoadParameter() prescribes
  if (!inContinuation())
    return;

  SNES snes = _arclength_nl->getSNES();

  // The type has to be set before any SNESNewtonAL call: those are function lookups on the solver
  // object that do nothing at all, and report nothing, on a solver of another type
  LibmeshPetscCall(SNESSetType(snes, SNESNEWTONAL));
  // Mandatory: libMesh hands SNESSolve no right hand side, so the callback is the only source of
  // the tangent load PETSc has
  LibmeshPetscCall(SNESNewtonALSetFunction(snes, arcLengthTangentLoad, this));
  // PETSc's documentation and its code disagree over which correction type is the default, so it
  // is always set here rather than left alone
  LibmeshPetscCall(SNESNewtonALSetCorrectionType(snes, _correction_type));

  // MOOSE's convergence test reads the whole continuation as a single nonlinear solve: it reads
  // the residual jump at the start of each increment as divergence, and it treats the iteration
  // budget PETSc inflates for the whole continuation as the budget for a single solve. PETSc's
  // default test measures one increment at a time. It does not use a context, so it is given none.
  LibmeshPetscCall(SNESSetConvergenceTest(
      snes, SNESConvergedDefault, LIBMESH_PETSC_NULLPTR, LIBMESH_PETSC_NULLPTR));

  // The update function signature carries no context, so the problem travels on the solver
  LibmeshPetscCall(SNESSetApplicationContext(snes, this));
  LibmeshPetscCall(SNESSetUpdate(snes, arcLengthUpdate));
}

void
ArcLengthProblem::onTimestepBegin()
{
  FEProblemBase::onTimestepBegin();

  if (!_transient)
    return;

  // The radius of an increment is 'step_size' itself, at every time step size: a cutback shrinks
  // the load span a step covers and leaves the turning radius alone, because the solution
  // excursion across a sharp turn of the path is set by the shape of the path and does not shrink
  // with the span, so a retry that shrank the radius with the span would confine the corrector
  // exactly when the ladder needs its reach.

  // The ceiling of the step local load parameter is the anchor of a step: 1, the end of the load
  // increment, however far the step was cut back. The floor is a fixed span of physical load
  // factor, sized against the nominal load increment scale of the input, which is the time step
  // size of the first step: a floor fixed in step local units would shrink the physical span with
  // every cutback, and the load drop of a sharp turn, which the shape of the path sets, would
  // stop fitting inside a step exactly when the retry ladder needs it to, while no floor at all
  // lets a corrector oscillation at such a turn run away by orders of magnitude. A thousand
  // nominal increments of unloading span clears the transient dips a corrector makes rounding
  // the sharpest turns, which reach far below what the step commits, and still catches an
  // observed runaway dozens of times over. SNESNEWTONAL reads its settings from the options
  // database, and the stored pairs reach that database once rather than once per solve, so the
  // per-attempt value is written to the database directly.
  if (_nominal_dt == 0.0)
    _nominal_dt = _dt;
  const Real floor = -std::max(1.0, 1000.0 * _nominal_dt / _dt);
  LibmeshPetscCall(PetscOptionsSetValue(
      LIBMESH_PETSC_NULLPTR, "-snes_newtonal_lambda_min", Moose::stringifyExact(floor).c_str()));

  // The time step size is the scale of the load increment: the step local load parameter spans it
  // over a range of 0 to 1, and the part of it the increment traverses is what the step commits.
  // The sign carries the direction of travel across the step boundary, which is where PETSc's own
  // predictor, whose memory of it ends with the solve, does not reach.
  _step_load_increment = _path_direction * _dt;
  _increment = 0;
  _ended_on_spent_budget = false;
  _retrace_handled = false;
}

void
ArcLengthProblem::onTimestepEnd()
{
  FEProblemBase::onTimestepEnd();

  if (!_transient)
    return;

  if (!_ended_on_spent_budget)
    return;

  // The path stopped where the budget ran out instead of at the end of the load increment of the
  // step, so what advanceState() commits is the part of that increment the path traced, signed by
  // the direction the step travelled in. _step_lambda is the step local parameter the load factor
  // reported from here on is composed with, so the load committed and the load reported match.
  _step_load_increment *= _step_lambda;

  // A step whose net change ran against its direction of travel turned around within itself, so
  // the way it came is the new forward. A step that made no net change at all keeps the direction,
  // there being nothing to read a turn off. This is resolved here rather than where the increment
  // is committed because _step_lambda does not survive a checkpoint and the direction has to.
  if (_step_lambda < 0)
    _path_direction = -_path_direction;
}

void
ArcLengthProblem::advanceState()
{
  FEProblemBase::advanceState();

  if (!_transient)
    return;

  // Only a converged step is advanced, so nothing else has to establish that the increment armed
  // for this step is one that was actually traced
  _lambda_accum += _step_load_increment;
  _step_load_increment = 0.0;
}

bool
ArcLengthProblem::solverSystemConverged(const unsigned int sys_num)
{
  // A prescribed step is judged the ordinary way; the budget exit and the dissipation judgement
  // belong to the continuation alone
  return inContinuation() ? continuationConverged(sys_num)
                          : FEProblemBase::solverSystemConverged(sys_num);
}

bool
ArcLengthProblem::continuationConverged(const unsigned int sys_num)
{
  bool converged = FEProblemBase::solverSystemConverged(sys_num);

  // The budget is a designed ending of a transient step always, its internal budget being the one
  // increment every step advances by, and of a one-shot path while the input says so
  if (!converged && (_transient || _end_on_max_continuation_steps))
  {
    // One publication happens at the top of every increment the path starts, so a budget spent in
    // full leaves _increment at exactly the budget, and a corrector that fails earlier stops the
    // count short of it. A corrector failing in the last permitted increment leaves the same
    // count as a spent budget; the iteration count below is what tells the two apart.
    //
    // libMesh destroys the SNES at the end of a solve and asking for it again would build a new
    // one, so the reason comes from the cache libMesh keeps of it.
    auto & solver = static_cast<PetscNonlinearSolver<Number> &>(*_arclength_nl->nonlinearSolver());
    const unsigned int budget = _transient ? 1 : getParam<unsigned int>("max_continuation_steps");
    if (_increment == budget && solver.get_converged_reason() == SNES_DIVERGED_MAX_IT)
    {
      // A corrector that burns out its iterations inside the last permitted increment ends with
      // the same reason and the same increment count as a spent budget, and the nonlinear
      // iteration count at the exit is what tells them apart: SNESSolve_NEWTONAL consults its
      // continuation budget only at the boundary a converged increment opens, so a budget stop
      // leaves the count wherever the converged increment finished, short of the iteration cap,
      // while a corrector stopped by the cap itself leaves the count sitting exactly at it. The
      // count restarts at every boundary publication, so it is per increment. Accepting a
      // burned-out corrector would commit a state that is not an equilibrium, whose
      // contamination the steps that follow inherit. The corner is an increment that converges
      // on exactly its last permitted iteration, which reads as a burnout here and is failed
      // toward a retry, the safe direction.
      const auto max_its = es().parameters.get<unsigned int>("nonlinear solver maximum iterations");
      const auto exit_its = _arclength_nl->nNonlinearIterations();
      if (exit_its < max_its)
      {
        // This is the one ending that leaves a transient step short of its whole load increment,
        // and it is told apart from the other one here alone
        _ended_on_spent_budget = true;
        converged = true;
      }
      else
        _console << "Arc length step spent its budget without converging an increment (the "
                    "final increment burned all "
                 << exit_its
                 << " nonlinear iterations), so the attempt is failed rather than committed off "
                    "equilibrium."
                 << std::endl;
    }
  }

  if (!converged)
    return false;

  // A step whose net load change runs downward is doing one of two things: tracing a descending
  // stretch of the path, which sheds load because the structure dissipates, or walking back down
  // an elastic branch, which sheds load without dissipating anything. The second converges just as
  // well as the first, because an elastic branch is made of equilibrium points, and it is where a
  // fresh predictor whose sign choice has no memory of the direction of travel strands a trace: at
  // a reflection it walks back the branch just traced, and at the foot of a serration it takes the
  // unloading half of the elastic line rather than the reloading half, either of which reads as a
  // descent that never ends. Every converged step is judged, whichever ending it converged
  // through, because a direction of travel turned the wrong way descends through whole load
  // increments that end as ordinary converged solves. Failing the attempt hands the step to the
  // TimeStepper like any other failure, and turning the direction of travel mirrors the tangent
  // load of the retry, which walks it the way the path was actually going.
  if (_transient && _step_lambda * _step_load_increment < 0 && !stepDissipated())
  {
    if (!_retrace_handled)
    {
      _retrace_handled = true;
      _path_direction = -_path_direction;
      _console << "Arc length step descended without dissipating, which is a walk back down an "
                  "elastic branch rather than a descent along the path, so the attempt is failed "
                  "and the retry travels the other way."
               << std::endl;
    }
    return false;
  }

  return true;
}

bool
ArcLengthProblem::stepDissipated()
{
  // The executioner asks for the convergence of a solve that has returned, so the solution holds
  // the state the step ended at and the change of the step is its distance from the old state
  auto change = _arclength_nl->solution().clone();
  *change -= _arclength_nl->solutionOld();

  // The load tag was assembled by the last residual evaluation of the solve, which is close enough
  // to the pattern at the start of the step for a measure that is only ever read against zero
  const auto & load = _arclength_nl->getVector(_arclength_nl->loadVectorTag());
  const Real pattern_dot_change = libmesh_real(load.dot(*change));
  const Real pattern_dot_start = libmesh_real(load.dot(_arclength_nl->solutionOld()));

  // The two terms of the dissipation increment of Verhoosel and de Borst, each carried separately
  // so that their cancellation can be read: the elastic unload of a linear structure is
  // proportional, which cancels them exactly, while a descent that dissipates leaves a remainder
  // on the order of the terms themselves. The measure is read against the size of its terms rather
  // than against a unit, because the load pattern carries the arbitrary scale of the input, and by
  // magnitude, because the assembled load tag carries the sign convention of the residual.
  const Real term_start = _lambda_accum * pattern_dot_change;
  const Real term_change = _step_lambda * _step_load_increment * pattern_dot_start;

  // A held load does work over the step that the two terms above never see: under one, elastic
  // unloading follows an affine line rather than a proportional ray, the cancellation breaks by
  // the work of the held load, and a walk back down an elastic branch would read as dissipative.
  // Its work restores the cancellation, taken from the tag the input routes the held loads to.
  Real term_held = 0.0;
  if (isParamValid("held_load_vector_tag"))
    term_held = libmesh_real(
        _arclength_nl->getVector(getVectorTagID(getParam<TagName>("held_load_vector_tag")))
            .dot(*change));

  const Real dissipation = 0.5 * (term_start + term_held - term_change);
  const Real scale = 0.5 * (std::abs(term_start) + std::abs(term_held) + std::abs(term_change));

  // The cancellation of a dissipation-free descent is imperfect, holding to about a part in a
  // thousand of the terms in practice, while a descent along the path leaves a remainder on the
  // order of the terms themselves, so the threshold sits between the two with an order of
  // magnitude of room on both sides
  return std::abs(dissipation) > 1e-2 * scale;
}

void
ArcLengthProblem::computeResidualSys(NonlinearImplicitSystem & sys,
                                     const NumericVector<Number> & soln,
                                     NumericVector<Number> & residual)
{
  FEProblemBase::computeResidualSys(sys, soln, residual);

  // A continuation reassembles the load tag in computeTangentLoad, which PETSc calls before every
  // residual. A prescribed step has no tangent load, so the tag is assembled here instead, at the
  // iterate the evaluation above already synced into the solution.
  if (!inContinuation())
  {
    computeResidualTags({_arclength_nl->loadVectorTag()});
    if (!_checked_load_on_constrained_dofs)
    {
      checkLoadOnConstrainedDofs();
      _checked_load_on_constrained_dofs = true;
    }
  }

  // The load objects assemble into the load tag during the evaluation above, which is what keeps
  // them out of the residual MOOSE composes. Scaling that tag by the load factor here makes the
  // residual PETSc solves with F_int + lambda * R_load, where a transient run composes the factor
  // out of the load the committed steps carry and the increment of the step being traced.
  // An exception that interrupts the assembly is handled inside it, with the solve set to stop
  // through the function domain error, but it can leave the tag half-assembled on the ranks the
  // interruption reached; the close is a no-op otherwise and keeps the composition from
  // aborting inside PETSc while the solve winds down
  auto & load = _arclength_nl->getVector(_arclength_nl->loadVectorTag());
  load.close();
  residual.add(updateLoadParameter(), load);
  residual.close();
}

void
ArcLengthProblem::computeJacobianSys(NonlinearImplicitSystem & sys,
                                     const NumericVector<Number> & soln,
                                     SparseMatrix<Number> & jacobian)
{
  FEProblemBase::computeJacobianSys(sys, soln, jacobian);

  // The derivative of the load with respect to the solution is assembled into the load matrix tag
  // during the evaluation above. Scaling it by the same load factor here keeps the Jacobian the
  // derivative of the residual composed in computeResidualSys.
  auto & load_jacobian = _arclength_nl->getMatrix(_arclength_nl->loadMatrixTag());
  // The matrix being added from has to be assembled
  load_jacobian.close();
  jacobian.add(updateLoadParameter(), load_jacobian);
  jacobian.close();
}

void
ArcLengthProblem::computeTangentLoad(Vec x, Vec q)
{
  TIME_SECTION("computeTangentLoad", 3);

  // PETSc evaluates the tangent load before the residual, so the load tag still holds the load at
  // the previous iterate and has to be assembled again here
  _arclength_nl->setSolution(localizeSolution(x));
  computeResidualTags({_arclength_nl->loadVectorTag()});

  if (!_checked_load_on_constrained_dofs)
  {
    checkLoadOnConstrainedDofs();
    _checked_load_on_constrained_dofs = true;
  }

  // PETSc's tangent load is Q = -dF/dlambda, with F the vector returned by the residual
  // evaluation: SNESSolve_NEWTONAL solves J * deltaX_Q = Q for the variation of the solution with
  // respect to the load parameter, and it builds Q that way for its own right hand side path,
  // where the residual gets lambda * b and Q gets b with a coefficient of one. F is
  // F_int + lambda * R_load here, so dF/dlambda is R_load and Q is the load tag negated, with no
  // factor of lambda. SNESSolve_NEWTONAL zeroes Q before the callback, so this is all of it. A
  // transient step carries the load parameter over its own increment, which puts that signed
  // increment on dF/dlambda: its magnitude makes the step local parameter trace the increment of
  // one step, and its sign makes an increasing step local parameter mean travel in the remembered
  // direction, which is the only thing carrying that direction across a step boundary.
  PetscVector<Number> tangent_load(q, _communicator);
  // closed for the same reason the residual composition closes it: an interrupted assembly may
  // have left the tag half-built while the solve winds down through the function domain error
  auto & load = _arclength_nl->getVector(_arclength_nl->loadVectorTag());
  load.close();
  tangent_load.add(_transient ? -_step_load_increment : -1.0, load);
}

void
ArcLengthProblem::checkLoadOnConstrainedDofs()
{
  const auto & nodal_bcs = _arclength_nl->getNodalBCWarehouse();
  if (!nodal_bcs.hasActiveObjects())
    return;

  const auto & load = _arclength_nl->getVector(_arclength_nl->loadVectorTag());
  const auto system_number = _arclength_nl->number();

  std::set<std::string> offenders;
  for (const auto & bnode : getCurrentAlgebraicBndNodeRange())
  {
    const Node & node = *bnode->_node;
    if (node.processor_id() != processor_id() ||
        !nodal_bcs.hasActiveBoundaryObjects(bnode->_bnd_id))
      continue;

    for (const auto & bc : nodal_bcs.getActiveBoundaryObjects(bnode->_bnd_id))
    {
      const auto & variable = bc->variable();
      for (const auto component : make_range(node.n_comp(system_number, variable.number())))
        if (load(node.dof_number(system_number, variable.number(), component)) != 0)
          offenders.insert("'" + bc->name() + "' on variable '" + variable.name() + "'");
    }
  }
  _communicator.set_union(offenders);

  if (offenders.size())
    mooseError("The arc-length load is nonzero at degrees of freedom that these nodal boundary "
               "conditions constrain: ",
               Moose::stringify(offenders, ", "),
               ". A nodal boundary condition sets its residual row after the load has been "
               "assembled, so the load applied there is dropped from the residual while the "
               "tangent load still carries it, and the continuation traces a path against a load "
               "the solve never applies. Either move the load off the constrained nodes, or "
               "enforce the constraint weakly with PenaltyDirichletBC.");
}

void
ArcLengthProblem::executeArcLengthIncrement(SNES snes)
{
  TIME_SECTION("executeArcLengthIncrement", 3);

  // SNESSolve_NEWTONAL calls the update function at the top of every corrector iteration, and it
  // zeroes the function norm once per continuation increment and sets it after every corrector
  // iteration. A zero norm is therefore the increment boundary, where the iterate and the load
  // parameter are those of the equilibrium point that just converged.
  PetscReal function_norm;
  LibmeshPetscCall(SNESGetFunctionNorm(snes, &function_norm));
  if (function_norm != 0)
    return;

  Vec x;
  LibmeshPetscCall(SNESGetSolution(snes, &x));
  _arclength_nl->setSolution(localizeSolution(x));

  updateLoadParameter();

  _console << "\nArc length increment " << _increment << ", lambda = " << _load_parameter << '\n'
           << std::endl;

  // PETSc numbers nonlinear iterations across the whole continuation, while SNESSolve_NEWTONAL
  // bounds its corrector and continuation loops with counters of its own, so the running number
  // restarts at the increment boundary and every block counts from zero. The budget ending is
  // judged by the count this leaves at the exit of the solve, so it is per increment by
  // construction
  LibmeshPetscCall(SNESSetIterationNumber(snes, 0));

  execute(EXEC_ARC_LENGTH_INCREMENT);

  // A transient run writes its output on the time the steps advance, and a pseudo time interleaved
  // with that would corrupt the sequence of an output, so only the objects executed above record a
  // transient path
  if (!_transient)
  {
    // The time of the solve does not move within the step the continuation is traced in, so the
    // increment index stands in for it and every increment is written as a frame of its own
    const Real solve_time = _time;
    const int solve_step = _t_step;
    _time = _increment;
    _t_step = static_cast<int>(_increment);

    outputStep(EXEC_ARC_LENGTH_INCREMENT);

    // Putting the time of the solve back leaves the output at the end of the step untouched
    _time = solve_time;
    _t_step = solve_step;
  }

  ++_increment;
}

Real
ArcLengthProblem::updateLoadParameter()
{
  // A prescribed step ramps the whole of its load increment: the load factor it solves at is the
  // one the step ends at, held through the Newton iterations, and the step local parameter is
  // left at one so that the step commits its full increment exactly as a completed continuation
  // step does
  if (!inContinuation())
  {
    _step_lambda = 1.0;
    _load_parameter = _lambda_accum + _step_load_increment;
    return _load_parameter;
  }

  SNES snes = _arclength_nl->getSNES();

  PetscBool continuing;
  LibmeshPetscCall(PetscObjectTypeCompare((PetscObject)snes, SNESNEWTONAL, &continuing));
  // The load parameter only exists while the arc-length solver owns the SNES. Evaluations made
  // outside the continuation, computing the variable scaling for one, see the unloaded problem.
  if (!continuing)
    return 0.0;

  PetscReal lambda;
  LibmeshPetscCall(SNESNewtonALGetLoadParameter(snes, &lambda));
  // PETSc restarts the load parameter at every solve, which is what makes it the fraction of a
  // transient step's own increment that has been applied. Keeping it is what lets a step that ends
  // short of that increment commit the same fraction the load factor below carries.
  _step_lambda = lambda;
  _load_parameter = _transient ? _lambda_accum + _step_lambda * _step_load_increment : lambda;

  return _load_parameter;
}

const NumericVector<Number> &
ArcLengthProblem::localizeSolution(Vec x)
{
  auto & sys = _arclength_nl->system();
  PetscVector<Number> & solution = *cast_ptr<PetscVector<Number> *>(sys.solution.get());
  PetscVector<Number> iterate(x, _communicator);

  // Use the system's update() to get a good local version of the parallel solution. This operation
  // does not modify the incoming iterate, it only localizes information from it into
  // sys.current_local_solution.
  iterate.swap(solution);
  sys.update();
  iterate.swap(solution);

  return *sys.current_local_solution;
}

#endif

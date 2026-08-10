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
      "step_size", "step_size > 0", "Arc length increment taken per continuation step.");
  params.addRangeCheckedParam<unsigned int>(
      "max_continuation_steps",
      100,
      "max_continuation_steps > 0",
      "Maximum number of continuation steps. The solve fails when the "
      "path does not reach 'lambda_max' within this many steps, which in "
      "a transient run fails the time step and lets the TimeStepper cut "
      "the load increment back and retry it. Set "
      "'end_on_max_continuation_steps' to end the path there instead.");
  params.addParam<Real>("lambda_max",
                        1.0,
                        "Load parameter the continuation stops at. This is the only criterion the "
                        "solver ends a completed path with unless "
                        "'end_on_max_continuation_steps' is set. A transient run traces the load "
                        "increment of a single step with a load parameter spanning 0 to 1, so this "
                        "has to be 1 there.");
  params.addParam<Real>("lambda_min",
                        0.0,
                        "Lower clamp on the load parameter. A step that would take the load "
                        "parameter below this value is truncated to it. This is not an exit "
                        "criterion, so a path that descends to this value stays there until "
                        "'max_continuation_steps' runs out. The clamp is on the parameter of the "
                        "path being traced, so a transient run clamps how far a step may unload "
                        "below the load factor it started from.");
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
  params.addParam<bool>("end_on_max_continuation_steps",
                        false,
                        "Whether a continuation that spends the whole of "
                        "'max_continuation_steps' ends the path successfully instead of failing "
                        "the solve, which is what the default does. Set this to trace a softening "
                        "branch, where the load parameter falls monotonically past the peak and "
                        "'lambda_max' is never reachable, so the step budget is the end of the "
                        "path. This is meant for single-solve path tracing: the exit is read off "
                        "the count of increments a continuation traced, and a run that solves more "
                        "than once carries that count past the budget and reports the solve as "
                        "failed. A transient run takes its load increment from the time step size "
                        "instead, so this is a one-shot setting.");

  params.addParamNamesToGroup("load_vector_tag load_matrix_tag", "Tagging");
  params.addParamNamesToGroup("step_size max_continuation_steps lambda_max lambda_min psi_squared "
                              "correction_type end_on_max_continuation_steps",
                              "Arc length continuation");

  return params;
}

ArcLengthProblem::ArcLengthProblem(const InputParameters & parameters)
  : FEProblemBase(parameters),
#if PETSC_RELEASE_GREATER_EQUALS(3, 22, 0)
    _correction_type(correctionType(getParam<MooseEnum>("correction_type"))),
    _end_on_max_continuation_steps(getParam<bool>("end_on_max_continuation_steps")),
    _increment(0),
    _checked_load_on_constrained_dofs(false),
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
  // and MOOSE applies them along with the rest on every solve.
  auto & pairs = getPetscOptions().pairs;
  pairs.emplace_back("-snes_newtonal_step_size",
                     Moose::stringifyExact(getParam<Real>("step_size")));
  pairs.emplace_back("-snes_newtonal_max_continuation_steps",
                     Moose::stringify(getParam<unsigned int>("max_continuation_steps")));
  pairs.emplace_back("-snes_newtonal_psisq", Moose::stringifyExact(getParam<Real>("psi_squared")));
  pairs.emplace_back("-snes_newtonal_lambda_min",
                     Moose::stringifyExact(getParam<Real>("lambda_min")));
  pairs.emplace_back("-snes_newtonal_lambda_max",
                     Moose::stringifyExact(getParam<Real>("lambda_max")));
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

  if (_transient && getParam<Real>("lambda_max") != 1.0)
    paramError("lambda_max",
               "'lambda_max' has to be 1 in a transient run. A time step there traces the load "
               "increment of that step alone, with a load parameter that spans the increment over "
               "a range of 0 to 1, and the load increment is set by the time step size instead. "
               "Control the load with the TimeStepper and the end_time.");

  if (_transient && _end_on_max_continuation_steps)
    paramError("end_on_max_continuation_steps",
               "'end_on_max_continuation_steps' is a path-tracing exit for the one-shot mode and "
               "is not available in a transient run. A transient step's load increment is set by "
               "its time step size, and the load factor the committed steps carry advances by that "
               "whole increment once the step ends, so a step cut short by a spent continuation "
               "budget would have applied part of its load increment while the load factor "
               "advanced all of it, and the load factor reported from there on would no longer be "
               "the load that was applied. Let such a step fail instead and let the TimeStepper "
               "cut the load increment back.");
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

void
ArcLengthProblem::initPetscOutputAndSomeSolverSettings()
{
  // Creates the SNES and puts the MOOSE defaults, including the convergence test replaced below,
  // on it
  FEProblemBase::initPetscOutputAndSomeSolverSettings();

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

  // Taking the time step size as the load increment is what makes the load factor a step ends at
  // the time it ends at, so cutting a step back cuts its load increment back in the same proportion
  _step_load_increment = _dt;
  _increment = 0;
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
  if (FEProblemBase::solverSystemConverged(sys_num))
    return true;

  if (!_end_on_max_continuation_steps)
    return false;

  // One publication happens at the top of every increment the path starts, so a budget spent in
  // full leaves _increment at exactly 'max_continuation_steps', and a corrector that fails earlier
  // stops the count short of it. Accepted corner: a corrector failing in the last permitted
  // increment leaves the same count as a spent budget and is reported here as a completed path.
  // The test golds the traced path, so a regression into that corner still fails its CSVDiff.
  if (_increment != getParam<unsigned int>("max_continuation_steps"))
    return false;

  // libMesh destroys the SNES at the end of a solve and asking for it again would build a new one,
  // so the reason comes from the cache libMesh keeps of it
  auto & solver = static_cast<PetscNonlinearSolver<Number> &>(*_arclength_nl->nonlinearSolver());
  return solver.get_converged_reason() == SNES_DIVERGED_MAX_IT;
}

void
ArcLengthProblem::computeResidualSys(NonlinearImplicitSystem & sys,
                                     const NumericVector<Number> & soln,
                                     NumericVector<Number> & residual)
{
  FEProblemBase::computeResidualSys(sys, soln, residual);

  // The load objects assemble into the load tag during the evaluation above, which is what keeps
  // them out of the residual MOOSE composes. Scaling that tag by the load factor here makes the
  // residual PETSc solves with F_int + lambda * R_load, where a transient run composes the factor
  // out of the load the committed steps carry and the increment of the step being traced.
  residual.add(updateLoadParameter(), _arclength_nl->getVector(_arclength_nl->loadVectorTag()));
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
  // transient step carries the load parameter over its own increment, which puts a factor of the
  // time step size on dF/dlambda and is what makes the step local parameter trace that increment.
  PetscVector<Number> tangent_load(q, _communicator);
  tangent_load.add(_transient ? -_step_load_increment : -1.0,
                   _arclength_nl->getVector(_arclength_nl->loadVectorTag()));
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
  // bounds its corrector and continuation loops with counters of its own, so the running number is
  // display state that can restart at the increment boundary and let every block count from zero
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
  // transient step's own increment that has been applied
  _load_parameter = _transient ? _lambda_accum + lambda * _step_load_increment : lambda;

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

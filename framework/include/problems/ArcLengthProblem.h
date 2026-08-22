//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "FEProblemBase.h"
#include "MooseEnum.h"

#include "libmesh/petsc_macro.h"

#include <petscsnes.h>

#include <optional>
#include <utility>
#include <vector>

class ArcLengthNonlinearSystem;
class NodalBCBase;

/**
 * Problem that traces an equilibrium path with arc-length continuation
 *
 * The residual is split into a standard part and a load part, and the load is scaled by a load
 * parameter lambda that is an unknown of the solve rather than a prescribed value:
 *
 * R(u, lambda) = F_int(u) + lambda * R_load(u) = 0
 *
 * A load object is designated by the two load tags, which travel together. The load vector tag,
 * put in the replacing parameter vector_tags, collects R_load. The load matrix tag, put in the
 * replacing parameter matrix_tags, collects its derivative with respect to the solution, which is
 * nonzero only for a deformation dependent, or follower, load. This problem composes the residual
 * and the Jacobian by scaling both tags with lambda, and PETSc's SNESNEWTONAL adds the arc-length
 * constraint that makes lambda solvable.
 *
 * A steady run traces the whole path within a single solve. A transient run steps along it,
 * advancing the trace by a single continuation increment per time step, which is the ordering a
 * history dependent material needs. Objects that record the path run on
 * EXEC_ARC_LENGTH_INCREMENT, which is executed once per increment.
 *
 * framework/doc/content/source/problems/ArcLengthProblem.md carries the rest: the softening
 * branch, the transient stepping and the direction of travel.
 *
 * PETSc added SNESNEWTONAL in 3.22.0, so this problem errors on an older PETSc.
 */
class ArcLengthProblem : public FEProblemBase
{
public:
  static InputParameters validParams();

  ArcLengthProblem(const InputParameters & parameters);

  /**
   * @return The load factor of the most recent continuation state, which a transient run reports
   * as the total the committed steps and the step being traced add up to so that it stays
   * continuous across steps
   */
  Real loadParameter() const { return _load_parameter; }

#if PETSC_RELEASE_GREATER_EQUALS(3, 22, 0)
  virtual void initialSetup() override;

  virtual void checkProblemIntegrity() override;

  /**
   * Makes the arc-length solver out of the SNES the base class creates and clears what the
   * previous traced path recorded. Everything set here is set again on every solve because libMesh
   * destroys the SNES at the end of each one, which also makes this the hook every solve passes
   * through and a fixed point iteration passes through more than once per time step.
   */
  virtual void initPetscOutputAndSomeSolverSettings() override;

  /**
   * Arms the transient step about to be solved: the load increment scale, which is the time step
   * size signed by the direction of travel, and the floor of the step local load parameter, which
   * is a fixed physical unloading span.
   *
   * The executioner reaches this before every attempt at a step, so a step the TimeStepper cuts
   * back arms the smaller load span of its retry, with the turning radius left at 'step_size'.
   */
  virtual void onTimestepBegin() override;

  /**
   * Narrows the armed load increment of a transient step down to the part of it the increment
   * traversed when the step ended on its spent internal budget, and turns the direction of travel
   * around when that part ran against it.
   *
   * The executioner reaches this after the solve of a step, before the objects executed at the end
   * of that step and before the checkpoint is written, so the load factor those objects report,
   * the increment a recovered run commits and the direction it resumes travelling in all belong to
   * the step that just ended. A fixed point iteration reaches it once per iteration, so the record
   * a solve leaves is consumed here and applied once.
   */
  virtual void onTimestepEnd() override;

  /**
   * Commits the load increment of the transient step that just converged along with the state that
   * step leaves behind.
   *
   * The executioner advances the state of a converged step only, so a step that was cut back and
   * retried commits the increment of its successful attempt alone, and the state it advances ahead
   * of the first step finds no increment armed to commit.
   */
  virtual void advanceState() override;

  /**
   * Reports a continuation that spent the whole of its step budget as converged where that budget
   * is a designed ending, which a transient step's internal budget of one increment always is and
   * a one-shot path's is while 'end_on_max_continuation_steps' holds, judges every committed
   * descent of a transient step by the energy it dissipates, and defers to the base class in every
   * other case.
   *
   * SNESSolve_NEWTONAL ends a spent budget and an increment that ran out of corrector iterations
   * with the same SNES_DIVERGED_MAX_IT. The count of increments the path started separates the two
   * everywhere but the last permitted increment, where the nonlinear iteration count at the exit
   * decides: a budget stop leaves it short of the iteration cap, a corrector stopped by the cap
   * itself sitting exactly at it. A burned-out corrector is failed toward a retry rather than
   * committed off equilibrium.
   *
   * @param sys_num The solver system to report the convergence of
   * @return Whether that system converged
   */
  virtual bool solverSystemConverged(const unsigned int sys_num) override;

  /**
   * Forms the residual the standard way and adds the load tag to it, scaled by the load factor
   * updateLoadParameter() makes, so that the solver is given F_int + lambda * R_load.
   */
  virtual void computeResidualSys(libMesh::NonlinearImplicitSystem & sys,
                                  const NumericVector<libMesh::Number> & soln,
                                  NumericVector<libMesh::Number> & residual) override;

  /**
   * Forms the Jacobian the standard way and adds the load matrix tag to it, scaled by the same
   * load factor, which is the derivative of the residual computeResidualSys forms. The rows the
   * nodal boundary conditions constrain are cleared out of the load matrix first, the standard
   * assembly having already reduced those rows to the constraints themselves.
   */
  virtual void computeJacobianSys(libMesh::NonlinearImplicitSystem & sys,
                                  const NumericVector<libMesh::Number> & soln,
                                  libMesh::SparseMatrix<libMesh::Number> & jacobian) override;
#endif

private:
#if PETSC_RELEASE_GREATER_EQUALS(3, 22, 0)
  /**
   * Fills the tangent load vector that PETSc asks for at each function evaluation.
   *
   * PETSc defines the tangent load as Q = -dF/dlambda, where F is the vector this problem returns
   * from its residual evaluation, and solves J * dX/dlambda = Q for the variation of the solution
   * with respect to the load parameter. F is F_int + lambda * R_load here, so Q is the load
   * residual with its sign flipped and carries no factor of lambda.
   *
   * A transient step carries the load parameter over its own increment, where dF/dlambda is
   * Delta * R_load with Delta the signed load increment of the step. The magnitude of Delta is
   * what makes the step local parameter, over its range of 0 to 1, trace the increment of that one
   * step, and its sign is what makes an increasing step local parameter mean travel in the
   * remembered direction.
   *
   * PETSc asks for the tangent load before the residual of the iterate it hands over, so the load
   * tag still holds the load at the previous iterate and is reassembled here.
   *
   * @param x The iterate to evaluate the load at. PETSc read locks it, so it is never written to
   * @param q The tangent load vector to fill. It arrives zeroed and this problem owns all of it
   */
  void computeTangentLoad(Vec x, Vec q);

  /**
   * Publishes the state of a converged continuation increment: syncs the iterate into the
   * solution, caches the load factor and the step local parameter it was reached at, and executes
   * on EXEC_ARC_LENGTH_INCREMENT, outputting there as well outside a transient run.
   *
   * A whole continuation is traced within a single step of the solve, so the index of the
   * increment stands in for the time while the increment is output and every increment is written
   * as a frame of its own. A transient run already writes its output on the time the steps
   * advance, and that pseudo time interleaved with it would corrupt the sequence of an output, so
   * a transient path is recorded by the objects executed here and no increment of one is output.
   *
   * @param snes The solver being updated
   */
  void executeArcLengthIncrement(SNES snes);

  /**
   * Assembles the load vector tag at the solution the system currently holds, and nothing else.
   *
   * PETSc asks for a load of its own before every residual, so this runs more often than an
   * ordinary residual evaluation does. It goes to the nonlinear system rather than to
   * FEProblemBase::computeResidualTags, which would run the transfers, the MultiApps and the
   * EXEC_LINEAR objects along with the assembly, and carries the state and the exception handling
   * that method puts around an assembly.
   *
   * The displaced mesh is moved to the solution the system holds before the assembly, so that a
   * load object with use_displaced_mesh = true is assembled on the current configuration.
   */
  void assembleLoadTag();

  /**
   * @return The local degrees of freedom the nodal boundary conditions constrain, each paired with
   * the boundary condition that constrains it
   */
  std::vector<std::pair<dof_id_type, const NodalBCBase *>> constrainedNodalDofs();

  /**
   * Errors when the assembled load residual is nonzero at a degree of freedom a nodal boundary
   * condition constrains.
   *
   * A nodal boundary condition overwrites its residual row after the load has been assembled, so a
   * load entry landing in such a row is dropped from the residual PETSc solves with while the
   * tangent load still carries it. The continuation then traces a path against a load it never
   * applies, with nothing to report it.
   *
   * Which rows the load reaches is set by the objects the input adds, so the check runs once and
   * every call after that returns. A load that is identically zero carries no pattern to examine,
   * so the check waits for the first assembled load that is not.
   */
  void checkLoadOnConstrainedDofs();

  /**
   * Reads the load parameter PETSc currently holds, makes the load factor the residual and the
   * Jacobian are composed with out of it, and caches that for loadParameter(). Evaluations made
   * outside the continuation, where the arc-length solver does not own the SNES and there is no
   * load parameter to read, report zero and leave the cache alone.
   *
   * PETSc restarts its load parameter at every solve, so a transient run reads it as the fraction
   * of the current step's increment that has been applied and adds the load factor the committed
   * steps carry. That fraction is kept so that a step ending short of its whole increment commits
   * the same fraction the load factor reported here is composed with.
   *
   * @return The load factor to compose the residual and the Jacobian with
   */
  Real updateLoadParameter();

  /**
   * Localizes a PETSc iterate into the solution that MOOSE assembles with and samples
   *
   * @param x The iterate to localize
   * @return The localized solution
   */
  const NumericVector<Number> & localizeSolution(Vec x);

  /**
   * The convergence decision of solverSystemConverged, separated out so that the outcome of every
   * attempt is recorded on its way back to the executioner, which is what a retry reads to decide
   * whether to shrink the continuation radius
   *
   * @param sys_num The solver system to report the convergence of
   * @return Whether that system converged
   */
  bool continuationConverged(const unsigned int sys_num);

  /**
   * @return Whether the solve being made is a continuation. A steady solve always is; a transient
   * step is while the controllable 'use_continuation' holds true, and otherwise it is an ordinary
   * Newton solve at the prescribed load factor the committed steps and the whole load increment of
   * the step add up to
   */
  bool inContinuation() const;

  /**
   * @return Whether the step that just converged dissipated energy, measured by the increment
   * Verhoosel and de Borst control their continuation with, carrying the work of the loads held
   * over the step alongside it: dtau = (lambda_0 * f^T * du + f_held^T * du - dlambda * f^T * u_0)
   * / 2, with f the load pattern, f_held the pattern of the loads routed to
   * 'held_load_vector_tag', which is zero where no tag is given, u_0 and lambda_0 the state and
   * load factor the step started from, and du and dlambda the changes the step made. The elastic
   * unload of a linear structure is proportional, which cancels the terms of dtau exactly, so the
   * magnitude of dtau against the magnitudes of its terms is what tells a descent along the path
   * from a walk back down an elastic branch, which converges just as well and which the outcome of
   * the solve alone cannot expose
   */
  bool stepDissipated() const;

  /**
   * Maps the 'correction_type' parameter onto the PETSc correction type
   *
   * @param correction_type The value of the 'correction_type' parameter
   * @return The PETSc correction type it names
   */
  static SNESNewtonALCorrectionType correctionType(const MooseEnum & correction_type);

  /**
   * PETSc callback that hands the tangent load to SNESNEWTONAL
   *
   * @param snes The solver asking for the tangent load, unused because the problem arrives as the
   * context
   * @param x The iterate to evaluate the load at
   * @param q The tangent load vector to fill
   * @param context The arc-length problem, given to PETSc with SNESNewtonALSetFunction
   */
  static PetscErrorCode arcLengthTangentLoad(SNES snes, Vec x, Vec q, void * context);

  /**
   * PETSc callback that runs at the top of every corrector iteration, where the arc-length problem
   * publishes an increment that has just converged
   *
   * @param snes The solver being updated, which carries the arc-length problem as its application
   * context
   * @param step The index of the corrector iteration, unused because the increment boundary is
   * read off the function norm instead
   */
  static PetscErrorCode arcLengthUpdate(SNES snes, PetscInt step);

  /// The nonlinear system that owns the load tags
  std::shared_ptr<ArcLengthNonlinearSystem> _arclength_nl;

  /// Vector tag of the loads held constant over a transient continuation, which is unset when
  /// 'held_load_vector_tag' is not given and no held load work enters the dissipation judgement
  std::optional<TagID> _held_load_vector_tag;

  /// Scheme PETSc corrects an iterate back onto the arc-length constraint surface with
  const SNESNewtonALCorrectionType _correction_type;

  /// Whether a continuation that spends the whole of its step budget ends the path successfully
  /// instead of failing the solve
  const bool _end_on_max_continuation_steps;

  /// Whether a transient step runs a continuation, with false solving the step as an ordinary
  /// prescribed ramp at the load factor the committed steps and the whole load increment of the
  /// step add up to. Held by reference so that a [Controls] object writing the controllable
  /// parameter is obeyed by the next solve
  const bool & _use_continuation;

  /// Whether the attempt being judged has already been failed as a dissipation-free descent, which
  /// keeps a repeated convergence query from turning the direction of travel a second time. Reset
  /// at the start of every solve, so every attempt may be failed that way once
  bool _retrace_handled;

  /// Index of the continuation increment being published, counted from the start of the path,
  /// which every solve restarts because each traces a path of its own
  unsigned int _increment;

  /// Whether the continuation of the transient step being solved ended on a spent step budget
  /// instead of at the end of the load increment of that step
  bool _ended_on_spent_budget;

  /// Whether the load has already been checked against the degrees of freedom the nodal BCs own
  bool _checked_load_on_constrained_dofs;

  /// Step local load parameter of the most recent continuation state, which is the fraction of the
  /// load increment scale of the transient step being solved that has been applied
  Real _step_lambda;

  /// Time step size of the first transient step, which is the nominal load increment scale the
  /// input chose. The clamps on the step local load parameter are sized against it so that how far
  /// a step may load or unload is a fixed span of physical load factor. Restartable so a recovered
  /// run keeps the same span
  Real & _nominal_dt;

  /// Direction the equilibrium path is being travelled in, which is 1 or -1 and signs the load
  /// increment of a step so that the step continues the way its predecessor was going. Restartable
  /// so that a recovered run resumes travelling the way it left off
  int & _path_direction;

  /// Load factor the committed transient steps carry, which is where the current step's increment
  /// starts. Restartable so that a recovered run resumes the path at the load it left it at
  Real & _lambda_accum;

  /// Signed load increment the transient step being solved applies, narrowed to the part of it the
  /// path traced when the step ends on a spent step budget, and zero once it has been committed.
  /// Restartable because a step commits its increment only after the checkpoint has been written
  Real & _step_load_increment;
#endif

  /// Load factor of the most recent continuation state
  Real & _load_parameter;
};

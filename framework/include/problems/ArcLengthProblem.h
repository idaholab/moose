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

class ArcLengthNonlinearSystem;

/**
 * Problem that traces an equilibrium path with arc-length continuation
 *
 * The residual is split into a standard part and a load part, and the load is scaled by a load
 * parameter lambda that is an unknown of the solve rather than a prescribed value:
 *
 * R(u, lambda) = F_int(u) + lambda * R_load(u) = 0
 *
 * A load is designated by putting the load vector tag in the replacing parameter vector_tags of an
 * ordinary residual object, and the load Jacobian of a deformation dependent (follower) load by
 * putting the load matrix tag in matrix_tags. PETSc's SNESNEWTONAL adds the arc-length constraint
 * that makes lambda solvable, and traces the whole path within a single solve, so equilibrium
 * points past a limit point, where a prescribed load has nothing to converge to, are reachable.
 *
 * Objects that record the path run on EXEC_ARC_LENGTH_INCREMENT, which is executed once per
 * continuation increment. An output that carries that flag in its execute_on writes a frame per
 * increment, stamped with the index of the increment, which turns the path into an animation.
 *
 * The load parameter the continuation ends at is reached after the last increment is published, so
 * a per-increment record stops one increment short of it and a postprocessor executed on
 * EXEC_TIMESTEP_END is what reports that final value.
 *
 * A softening branch never climbs back to 'lambda_max': past the peak the load parameter falls
 * monotonically, so running out of continuation steps is the only end such a path has. The step
 * budget is therefore a designed end of the path by default, and a solve that spends the whole of
 * it is reported as a converged one. Setting 'end_on_max_continuation_steps' to false takes that
 * ending away and leaves 'lambda_max' as the only one a completed path has.
 *
 * A transient run steps along the path instead of tracing the whole of it in one solve: every time
 * step advances the trace by a single continuation increment, whose load change may carry either
 * sign, and commits the state that increment reaches. Stateful materials therefore advance along
 * the path itself and irreversible behavior accumulates increment by increment, including down a
 * descending branch. That ordering is what a history dependent material needs: a solve that
 * crosses a falling stretch of path in one span evaluates the history only at the state it ends
 * at, so the excursion never registers and the trace it commits is stiffer than the path it
 * claims to have crossed. This stepping is the classical incremental arc-length method of Riks
 * and Crisfield, with PETSc's solver as the engine of each increment.
 *
 * The load parameter PETSc solves for is step local: the residual of a step is
 * F_int + (Lambda_accum + lambda * Delta) * R_load, with Lambda_accum the load factor the
 * committed steps carry and Delta the time step size signed by the direction of travel. The
 * committed load factor moves by the part of Delta the increment traversed, which may be any
 * fraction of it and either sign, so the accumulated load factor is free to fall where the path
 * descends and time does not measure it: time is a pseudo parameter that counts arc steps, and
 * the load factor of the trace is what the load parameter postprocessor reports.
 *
 * The arc length of the increment a step takes is 'step_size' at every time step size: the
 * cutback that follows a failed step shrinks the load span the retry covers and leaves the
 * turning radius alone, because the solution excursion across a sharp turn of the path is set by
 * the shape of the path and does not shrink with the span. 'psi_squared' and 'correction_type'
 * govern each increment as they govern a one-shot continuation; 'max_continuation_steps',
 * 'end_on_max_continuation_steps', 'lambda_max' and 'lambda_min' belong to the one-shot path
 * alone and a transient run owns their values.
 *
 * The direction of travel has to survive a step boundary, and PETSc's predictor cannot carry it
 * there: its memory ends with the solve, and a fresh solve chooses the sign of its predictor from
 * the state it opens at, which near a limit point can walk a step backward along the branch just
 * traced. Such a step converges, because the branch it retraces is made of equilibrium points, so
 * the outcome of the solve cannot expose it and the energy the step dissipates is what does: a
 * descent along the path of a dissipative structure sheds load because the structure dissipates,
 * while a walk back down an elastic branch sheds load without dissipating anything. A step that
 * descends without dissipating is therefore failed, and its retry travels the other way. The
 * problem remembers a direction of travel of plus or minus one that signs Delta, a step whose
 * committed change ran against it turns it around, and the memory is restartable so that a
 * recovered run resumes travelling the way it left off.
 *
 * A transient path is not output per increment. The increment index stands in for the time while
 * an increment is output, and that pseudo time written alongside the time the steps advance would
 * corrupt the sequence of an output, so only the objects executed on EXEC_ARC_LENGTH_INCREMENT
 * record a transient path.
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
   * Makes the arc-length solver out of the SNES the base class creates. Everything set here is set
   * again on every solve because libMesh destroys the SNES at the end of each one.
   */
  virtual void initPetscOutputAndSomeSolverSettings() override;

  /**
   * Arms the transient step about to be solved: arms the load increment scale, which is the time
   * step size signed by the direction of travel, sets the floor of the step local load parameter
   * to the fixed physical unloading span, and clears the records the previous attempt left.
   *
   * The executioner reaches this before every attempt at a step, so a step the TimeStepper cuts
   * back arms the smaller load span of its retry, with the turning radius left at 'step_size'.
   */
  virtual void onTimestepBegin() override;

  /**
   * Narrows the armed load increment of a transient step down to the part of it the increment
   * traversed when the step ended on its spent internal budget, leaving a step whose increment
   * reached the end of its load span with the whole of it, and turns the direction of travel
   * around when the traversed part ran against it.
   *
   * The executioner reaches this after the solve of a step, before the objects executed at the end
   * of that step and before the checkpoint is written, so the load factor those objects report, the
   * increment a recovered run commits and the direction it resumes travelling in all belong to the
   * step that just ended.
   */
  virtual void onTimestepEnd() override;

  /**
   * Commits the load increment of the transient step that just converged along with the state that
   * step leaves behind.
   *
   * The executioner advances the state of a converged step only, so a step that was cut back and
   * retried commits the increment of its successful attempt alone, and the state it advances ahead
   * of the first step finds no increment armed to commit. It advances the state after the objects
   * executed at the end of a step have run, so those still report the load factor the step ended
   * at rather than the one the next step starts from.
   */
  virtual void advanceState() override;

  /**
   * Reports a continuation that spent the whole of its step budget as converged where that budget
   * is a designed ending, which a transient step's internal budget of one increment always is and
   * a one-shot path's is while 'end_on_max_continuation_steps' holds, judges every committed
   * descent of a transient step by the energy it dissipates, and defers to the base class in
   * every other case.
   *
   * SNESSolve_NEWTONAL ends a spent budget with SNES_DIVERGED_MAX_IT, which is also the reason it
   * ends an increment that ran out of corrector iterations with, so the reason alone does not say
   * a path was traced to the end of its budget. The count of increments the path started is what
   * separates the two, because only a budget spent in full reaches the count it allows.
   *
   * This is where the two endings a transient step has are told apart, so it records the one it
   * reports for onTimestepEnd().
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
   * load factor, which is the derivative of the residual computeResidualSys forms.
   */
  virtual void computeJacobianSys(libMesh::NonlinearImplicitSystem & sys,
                                  const NumericVector<libMesh::Number> & soln,
                                  libMesh::SparseMatrix<libMesh::Number> & jacobian) override;
#endif

protected:
#if PETSC_RELEASE_GREATER_EQUALS(3, 22, 0)
  /**
   * Fills the tangent load vector that PETSc asks for at each function evaluation.
   *
   * PETSc defines the tangent load as Q = -dF/dlambda, where F is the vector this problem returns
   * from its residual evaluation: it solves J * dX/dlambda = Q for the variation of the solution
   * with respect to the load parameter. F is F_int + lambda * R_load here, so dF/dlambda is
   * R_load and Q is the load residual with its sign flipped. Q carries no factor of lambda; PETSc
   * applies the load parameter itself wherever it needs the load at the current point.
   *
   * A transient step carries the load parameter over its own increment, where F is
   * F_int + (Lambda_accum + lambda * Delta) * R_load with Delta the signed load increment of the
   * step, so dF/dlambda is Delta * R_load and the tangent load is scaled by it. The magnitude of
   * Delta is what makes the step local parameter, over its range of 0 to 1, trace the increment of
   * that one step, and its sign is what makes an increasing step local parameter mean travel in
   * the remembered direction, so a step whose predecessor was descending descends as well.
   *
   * The load tag is reassembled here with FEProblemBase::computeResidualTags, which runs the
   * transfers, the MultiApps and the EXEC_LINEAR objects and controls along with the assembly. All
   * of those therefore run once more per nonlinear iteration than they do in an ordinary solve.
   *
   * @param x The iterate to evaluate the load at. PETSc read locks it, so it is never written to
   * @param q The tangent load vector to fill. It arrives zeroed and this problem owns all of it
   */
  void computeTangentLoad(Vec x, Vec q);

  /**
   * Publishes the state of a converged continuation increment: syncs the iterate into the
   * solution, caches the load factor, records the step local parameter against the deepest one the
   * step has reached, and executes on EXEC_ARC_LENGTH_INCREMENT, outputting there as well outside a
   * transient run.
   *
   * A whole continuation is traced within a single step of the solve, so every increment would be
   * output at the one time of that step and an output holding a frame per increment would keep
   * writing the same frame. The index of the increment, which only ever grows, stands in for the
   * time while the increment is output, and the time of the solve is put back before the solve
   * resumes so that the output at the end of the step is unaffected.
   *
   * A transient run already writes its output on the time the steps advance, and that pseudo time
   * interleaved with it would corrupt the sequence of an output, so a transient path is recorded
   * by the objects executed here and no increment of one is output.
   *
   * @param snes The solver being updated
   */
  void executeArcLengthIncrement(SNES snes);

  /**
   * Errors when the assembled load residual is nonzero at a degree of freedom a nodal boundary
   * condition constrains.
   *
   * A nodal boundary condition overwrites its residual row after the load has been assembled, so a
   * load entry landing in such a row is dropped from the residual PETSc solves with while the
   * tangent load still carries it. The continuation then traces a path against a load it never
   * applies, with nothing to report it.
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

  /// The nonlinear system that owns the load tags
  std::shared_ptr<ArcLengthNonlinearSystem> _arclength_nl;

  /// Scheme PETSc corrects an iterate back onto the arc-length constraint surface with
  const SNESNewtonALCorrectionType _correction_type;

  /// Whether a continuation that spends the whole of its step budget ends the path successfully
  /// instead of failing the solve
  const bool _end_on_max_continuation_steps;

  /// Whether a transient step runs a continuation, with false solving the step as an ordinary
  /// prescribed ramp at a load factor equal to the time. Controllable, and held by reference so
  /// that a [Controls] object writing the parameter is obeyed by the next solve, which replaces
  /// the checkpoint restart between a plain ramp and a continuation started from it
  const bool & _use_continuation;

  /// Whether the attempt being judged has already been failed as a dissipation-free descent, which
  /// keeps a repeated convergence query from turning the direction of travel a second time. Reset
  /// when a step is armed, so every attempt may be failed that way once
  bool _retrace_handled;

  /// Index of the continuation increment being published, counted from the start of the path,
  /// which a transient run restarts at every step because each traces a path of its own
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
  /// input chose. The clamps on the step local load parameter are sized against it so that how
  /// far a step may load or unload is a fixed span of physical load factor, however far the
  /// TimeStepper has cut the step back. Restartable so a recovered run keeps the same span
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

private:
#if PETSC_RELEASE_GREATER_EQUALS(3, 22, 0)
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
   * Newton solve at a prescribed load factor equal to the time
   */
  bool inContinuation() const;

  /**
   * @return Whether the step that just converged dissipated energy, measured by the increment
   * Verhoosel and de Borst control their continuation with: dtau = (lambda_0 * f^T * du
   * - dlambda * f^T * u_0) / 2, with f the load pattern, u_0 and lambda_0 the state and load
   * factor the step started from, and du and dlambda the changes the step made. A descending
   * stretch of the path of a damaging structure sheds load because the structure dissipates, while
   * unloading it elastically, with the damage held by its irreversibility, descends without
   * dissipating: the elastic unload of a linear structure is proportional, which cancels the two
   * terms of dtau exactly. The magnitude of dtau against the magnitudes of its terms is therefore
   * what tells a descent along the path from a walk back down an elastic branch, which converges
   * just as well and which the outcome of the solve alone cannot expose
   */
  bool stepDissipated();

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
#endif
};

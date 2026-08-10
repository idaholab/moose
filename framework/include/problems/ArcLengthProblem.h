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
 * PETSc added SNESNEWTONAL in 3.22.0, so this problem errors on an older PETSc.
 */
class ArcLengthProblem : public FEProblemBase
{
public:
  static InputParameters validParams();

  ArcLengthProblem(const InputParameters & parameters);

  /**
   * @return The load parameter lambda of the most recent continuation state
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
   * Forms the residual the standard way and adds the load tag to it, scaled by the load
   * parameter, so that the solver is given F_int + lambda * R_load.
   */
  virtual void computeResidualSys(libMesh::NonlinearImplicitSystem & sys,
                                  const NumericVector<libMesh::Number> & soln,
                                  NumericVector<libMesh::Number> & residual) override;

  /**
   * Forms the Jacobian the standard way and adds the load matrix tag to it, scaled by the load
   * parameter, which is the derivative of the residual computeResidualSys forms.
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
   * solution, caches the load parameter, and executes and outputs on EXEC_ARC_LENGTH_INCREMENT.
   *
   * A whole continuation is traced within a single step of the solve, so every increment would be
   * output at the one time of that step and an output holding a frame per increment would keep
   * writing the same frame. The index of the increment, which only ever grows, stands in for the
   * time while the increment is output, and the time of the solve is put back before the solve
   * resumes so that the output at the end of the step is unaffected.
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
   * Reads the load parameter PETSc currently holds and caches it for loadParameter(). Evaluations
   * made outside the continuation, where the arc-length solver does not own the SNES and there is
   * no load parameter to read, report zero and leave the cache alone.
   *
   * @return The load parameter to compose the residual and the Jacobian with
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

  /// Index of the continuation increment being published, counted from the start of the path
  unsigned int _increment;

  /// Whether the load has already been checked against the degrees of freedom the nodal BCs own
  bool _checked_load_on_constrained_dofs;
#endif

  /// Load parameter of the most recent continuation state
  Real & _load_parameter;

private:
#if PETSC_RELEASE_GREATER_EQUALS(3, 22, 0)
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

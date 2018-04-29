//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ActuallyExplicitEuler.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"
#include "LStableDirk4.h"

// libMesh includes
#include "libmesh/sparse_matrix.h"
#include "libmesh/nonlinear_solver.h"
#include "libmesh/preconditioner.h"

registerMooseObject("MooseApp", ActuallyExplicitEuler);

template <>
InputParameters
validParams<ActuallyExplicitEuler>()
{
  InputParameters params = validParams<TimeIntegrator>();

  MooseEnum solve_type("consistent lumped lump_preconditioned", "consistent");

  params.addParam<MooseEnum>(
      "solve_type",
      solve_type,
      "The way to solve the system.  A 'consistent' solve uses the full mass matrix and actually "
      "needs to use a linear solver to solve the problem.  'lumped' uses a lumped mass matrix with "
      "a simple inversion - incredibly fast but may be less accurate.  'lump_preconditioned' uses "
      "the lumped mass matrix as a preconditioner for the 'consistent' solve");

  params.addClassDescription(
      "Implementation of Explicit/Forward Euler without invoking any of the nonlinear solver");

  return params;
}

/**
 * Helper class to apply preconditioner
 */
class LumpedPreconditioner : public Preconditioner<Real>
{
public:
  LumpedPreconditioner(const NumericVector<Real> & diag_inverse)
    : Preconditioner(diag_inverse.comm()), _diag_inverse(diag_inverse)
  {
  }

  virtual void init() override
  {
    // No more initialization needed here
    _is_initialized = true;
  }

  virtual void apply(const NumericVector<Real> & x, NumericVector<Real> & y) override
  {
    y.pointwise_mult(_diag_inverse, x);
  }

protected:
  /// The inverse of the diagonal of the lumped matrix
  const NumericVector<Real> & _diag_inverse;
};

ActuallyExplicitEuler::ActuallyExplicitEuler(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _solve_type(getParam<MooseEnum>("solve_type")),
    _explicit_euler_update(_nl.addVector("explicit_euler_update", false, PARALLEL)),
    _mass_matrix_diag(_nl.addVector("mass_matrix_diag", false, PARALLEL))
{
  _Ke_time_tag = _fe_problem.getMatrixTagID("TIME");

  // Try to keep MOOSE from doing any nonlinear stuff
  _fe_problem.solverParams()._type = Moose::ST_LINEAR;

  if (_solve_type == CONSISTENT || _solve_type == LUMP_PRECONDITIONED)
    _linear_solver = LinearSolver<Number>::build(comm());

  if (_solve_type == LUMP_PRECONDITIONED)
  {
    _preconditioner = libmesh_make_unique<LumpedPreconditioner>(_mass_matrix_diag);
    _linear_solver->attach_preconditioner(_preconditioner.get());
    _linear_solver->init();
  }

  if (_solve_type == LUMPED || _solve_type == LUMP_PRECONDITIONED)
    _ones = &_nl.addVector("ones", false, PARALLEL);
}

void
ActuallyExplicitEuler::init()
{
  if (_solve_type == LUMPED || _solve_type == LUMP_PRECONDITIONED)
    *_ones = 1.;

  if (_solve_type == CONSISTENT || _solve_type == LUMP_PRECONDITIONED)
    Moose::PetscSupport::setLinearSolverDefaults(_fe_problem, *_linear_solver);
}

void
ActuallyExplicitEuler::preSolve()
{
}

void
ActuallyExplicitEuler::computeTimeDerivatives()
{
  _u_dot = *_solution;
  _u_dot -= _solution_old;
  _u_dot *= 1 / _dt;
  _u_dot.close();

  _du_dot_du = 1.0 / _dt;
}

void
ActuallyExplicitEuler::solve()
{
  // This ensures that all the Output objects in the OutputWarehouse
  // have had solveSetup() called, and sets the default solver
  // parameters for PETSc.
  _fe_problem.initPetscOutput();

  auto & nonlinear_system = _fe_problem.getNonlinearSystemBase();

  auto & libmesh_system = dynamic_cast<NonlinearImplicitSystem &>(nonlinear_system.system());

  auto Re_non_time_tag = nonlinear_system.nonTimeVectorTag();

  auto & mass_matrix = *libmesh_system.matrix;

  // Must compute the residual first
  _fe_problem.computeResidualTag(
      *libmesh_system.current_local_solution, _Re_non_time, Re_non_time_tag);

  _Re_non_time *= -1.;

  _fe_problem.computeJacobianTag(*libmesh_system.current_local_solution, mass_matrix, _Ke_time_tag);

  switch (_solve_type)
  {
    case CONSISTENT:
      _linear_solver->solve(mass_matrix, _explicit_euler_update, _Re_non_time, 1e-6, 100);
      break;

    case LUMPED:
      // Computes the sum of each row (lumping)
      // Note: This is actually how PETSc does it
      // It's not "perfectly optimal" - but it will be fast (and universal)
      mass_matrix.vector_mult(_mass_matrix_diag, *_ones);

      // "Invert" the diagonal mass matrix
      _mass_matrix_diag.reciprocal();

      // Multiply the inversion by the RHS
      _explicit_euler_update.pointwise_mult(_mass_matrix_diag, _Re_non_time);
      break;

    case LUMP_PRECONDITIONED:
      mass_matrix.vector_mult(_mass_matrix_diag, *_ones);
      _mass_matrix_diag.reciprocal();

      _linear_solver->solve(mass_matrix, _explicit_euler_update, _Re_non_time, 1e-6, 100);
      break;

    default:
      mooseError("Unknown solve_type in ActuallyExplicitEuler ");
  }

  *libmesh_system.solution = nonlinear_system.solutionOld();
  *libmesh_system.solution += _explicit_euler_update;

  libmesh_system.update();

  libmesh_system.nonlinear_solver->converged = true;
}

void
ActuallyExplicitEuler::postResidual(NumericVector<Number> & /* residual */)
{
}

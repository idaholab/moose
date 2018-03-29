//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LumpedExplicitEuler.h"
#include "NonlinearSystem.h"
#include "FEProblem.h"
#include "PetscSupport.h"
#include "LStableDirk4.h"

// libMesh includes
#include "libmesh/sparse_matrix.h"
#include "libmesh/petsc_linear_solver.h"
#include "libmesh/nonlinear_solver.h"

registerMooseObject("MooseApp", LumpedExplicitEuler);

template <>
InputParameters
validParams<LumpedExplicitEuler>()
{
  InputParameters params = validParams<TimeIntegrator>();

  return params;
}

LumpedExplicitEuler::LumpedExplicitEuler(const InputParameters & parameters)
  : TimeIntegrator(parameters),
    _explicit_euler_update(_nl.addVector("explicit_euler_update", false, PARALLEL))
{
  _Ke_time_tag = _fe_problem.addMatrixTag("TIME");
}

void
LumpedExplicitEuler::preSolve()
{
}

void
LumpedExplicitEuler::computeTimeDerivatives()
{
  _u_dot = *_solution;
  _u_dot -= _solution_old;
  _u_dot *= 1 / _dt;
  _u_dot.close();

  _du_dot_du = 1.0 / _dt;
}

void
LumpedExplicitEuler::solve()
{
  // This ensures that all the Output objects in the OutputWarehouse
  // have had solveSetup() called, and sets the default solver
  // parameters for PETSc.
  _fe_problem.initPetscOutput();

  auto & nonlinear_system = _fe_problem.getNonlinearSystemBase();

  auto & libmesh_system = dynamic_cast<NonlinearImplicitSystem &>(nonlinear_system.system());

  auto Re_non_time_tag = nonlinear_system.nonTimeVectorTag();
  // auto system_matrix_tag = nonlinear_system.systemMatrixTag();

  auto & mass_matrix = *libmesh_system.matrix;

  mass_matrix.close();

  // nonlinear_system.associateMatrixToTag(, _Ke_time_tag);

  std::cout << "Computing Jacobian!!" << std::endl;

  _fe_problem.computeJacobianTag(*libmesh_system.solution, mass_matrix, _Ke_time_tag);

  std::cout << "Finished Computing Jacobian!!" << std::endl;

  _fe_problem.computeResidualTag(*libmesh_system.solution, _Re_non_time, Re_non_time_tag);

  _Re_non_time *= -1.;

  PetscLinearSolver<Real> petsc_solver(comm());

  petsc_solver.solve(mass_matrix, _explicit_euler_update, _Re_non_time, 1e-6, 100);

  *libmesh_system.solution = nonlinear_system.solutionOld();
  *libmesh_system.solution += _explicit_euler_update;

  libmesh_system.update();

  libmesh_system.nonlinear_solver->converged = true;
}

void
LumpedExplicitEuler::postResidual(NumericVector<Number> & /* residual */)
{
}

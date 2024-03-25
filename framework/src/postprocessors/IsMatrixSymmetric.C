//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IsMatrixSymmetric.h"

#include "FEProblem.h"
#include "NonlinearSystemBase.h"

#include "libmesh/nonlinear_solver.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/petsc_matrix.h"

#include <petscmat.h>

registerMooseObject("MooseApp", IsMatrixSymmetric);

InputParameters
IsMatrixSymmetric::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Report whether the system matrix is symmetric or not.");
  params.addParam<Real>("symmetry_tol", 1e-8, "The tolerance for comparing symmetry");
  return params;
}

IsMatrixSymmetric::IsMatrixSymmetric(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _symm_tol(getParam<Real>("symmetry_tol"))
{
}

void
IsMatrixSymmetric::initialSetup()
{
  auto * const nl_solver = _fe_problem.getNonlinearSystemBase(0).nonlinearSolver();
  mooseAssert(nl_solver, "This should be non-null");
  _petsc_matrix = dynamic_cast<PetscMatrix<Number> *>(&nl_solver->system().get_system_matrix());
  if (!_petsc_matrix)
    mooseError("This postprocessor requires using PETSc as the solver backend");
}

Real
IsMatrixSymmetric::getValue() const
{
  PetscBool symmetric;
  auto ierr = MatIsSymmetric(_petsc_matrix->mat(), _symm_tol, &symmetric);
  LIBMESH_CHKERR(ierr);
  return symmetric;
}

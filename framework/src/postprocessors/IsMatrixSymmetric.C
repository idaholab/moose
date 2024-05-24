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
  params.addParam<Real>(
      "symmetry_tol", 1e-8, "The tolerance (both relative and absolute) for comparing symmetry");
  return params;
}

IsMatrixSymmetric::IsMatrixSymmetric(const InputParameters & parameters)
  : GeneralPostprocessor(parameters), _symm_tol(getParam<Real>("symmetry_tol"))
{
}

void
IsMatrixSymmetric::initialSetup()
{
  _mat_transpose = SparseMatrix<Number>::build(_communicator);
}

void
IsMatrixSymmetric::execute()
{
  _equiv = true;

  auto * const nl_solver = _fe_problem.getNonlinearSystemBase(0).nonlinearSolver();
  mooseAssert(nl_solver, "This should be non-null");
  auto & sys_mat = nl_solver->system().get_system_matrix();
  sys_mat.get_transpose(*_mat_transpose);

  for (const auto i : make_range(sys_mat.row_start(), sys_mat.row_stop()))
    for (const auto j : make_range(sys_mat.col_start(), sys_mat.col_stop()))
    {
      const auto val1 = sys_mat(i, j);
      const auto val2 = (*_mat_transpose)(i, j);
      if (!MooseUtils::relativeFuzzyEqual(val1, val2, _symm_tol) &&
          !MooseUtils::absoluteFuzzyEqual(val1, val2, _symm_tol))
      {
        _equiv = false;
        return;
      }
    }
}

void
IsMatrixSymmetric::finalize()
{
  _communicator.min(_equiv);
}

Real
IsMatrixSymmetric::getValue() const
{
  return _equiv;
}

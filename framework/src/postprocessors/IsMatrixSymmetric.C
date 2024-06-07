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
  params.addClassDescription("Report whether a matrix is symmetric or not.");
  params.addParam<Real>(
      "symmetry_tol", 1e-8, "The tolerance (both relative and absolute) for comparing symmetry");
  params.addParam<FileName>("binary_mat",
                            "",
                            "If supplied this matrix is tested for symmetry. If not supplied, the "
                            "system matrix is used.");
  return params;
}

IsMatrixSymmetric::IsMatrixSymmetric(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _symm_tol(getParam<Real>("symmetry_tol")),
    _mat_file(getParam<FileName>("binary_mat"))
{
}

IsMatrixSymmetric::~IsMatrixSymmetric()
{
  auto ierr = MatDestroy(&_binary_mat);
  CHKERRABORT(_communicator.get(), ierr);
  ierr = PetscViewerDestroy(&_binary_matviewer);
  CHKERRABORT(_communicator.get(), ierr);
}

void
IsMatrixSymmetric::initialSetup()
{
  _mat_transpose = SparseMatrix<Number>::build(_communicator);
  if (!_mat_file.empty())
  {
    auto ierr = MatCreate(_communicator.get(), &_binary_mat);
    LIBMESH_CHKERR(ierr);
    ierr = PetscViewerBinaryOpen(
        _communicator.get(), _mat_file.c_str(), FILE_MODE_READ, &_binary_matviewer);
    LIBMESH_CHKERR(ierr);

    _wrapped_petsc_mat = std::make_unique<PetscMatrix<Number>>(_binary_mat, _communicator);
    _mat = _wrapped_petsc_mat.get();
  }
  else
  {
    auto * const nl_solver = _fe_problem.getNonlinearSystemBase(0).nonlinearSolver();
    mooseAssert(nl_solver, "This should be non-null");
    auto & sys_mat = nl_solver->system().get_system_matrix();
    _mat = &sys_mat;
  }

  GeneralPostprocessor::initialSetup();
}

void
IsMatrixSymmetric::execute()
{
  if (_binary_mat)
  {
    auto ierr = MatLoad(_binary_mat, _binary_matviewer);
    LIBMESH_CHKERR(ierr);
  }
  _mat->get_transpose(*_mat_transpose);

  _equiv = true;
  for (const auto i : make_range(_mat->row_start(), _mat->row_stop()))
    for (const auto j : make_range(_mat->col_start(), _mat->col_stop()))
    {
      const auto val1 = (*_mat)(i, j);
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

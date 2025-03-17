//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addParam<std::string>("mat",
                               "The petsc binary mat file containing the matrix. If this "
                               "parameter is not provided, then the system matrix is used");
  params.addParam<Real>(
      "symmetry_tol", 1e-8, "The tolerance (both relative and absolute) for comparing symmetry");
  return params;
}

IsMatrixSymmetric::IsMatrixSymmetric(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _mat_from_file(isParamValid("mat")),
    _mat_file_name(_mat_from_file ? getParam<std::string>("mat") : ""),
    _symm_tol(getParam<Real>("symmetry_tol")),
    _equiv(true)
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
  // Pointer to the matrix we are analyzing for symmetry
  const SparseMatrix<Number> * mat;
  // Petsc matrix from file
  Mat file_mat;

  std::unique_ptr<SparseMatrix<Number>> file_mat_wrapper;
  if (_mat_from_file)
  {
    file_mat_wrapper =
        Moose::PetscSupport::createMatrixFromFile(_communicator, file_mat, _mat_file_name);
    mat = file_mat_wrapper.get();
  }
  else
  {
    auto * const nl_solver = _fe_problem.getNonlinearSystemBase(0).nonlinearSolver();
    mooseAssert(nl_solver, "This should be non-null");
    auto & sys_mat = nl_solver->system().get_system_matrix();
    mat = &sys_mat;
  }

  mat->get_transpose(*_mat_transpose);

  for (const auto i : make_range(mat->row_start(), mat->row_stop()))
    for (const auto j : make_range(mat->col_start(), mat->col_stop()))
    {
      const auto val1 = (*mat)(i, j);
      const auto val2 = (*_mat_transpose)(i, j);
      if (!MooseUtils::relativeFuzzyEqual(val1, val2, _symm_tol) &&
          !MooseUtils::absoluteFuzzyEqual(val1, val2, _symm_tol))
      {
        _equiv = false;
        goto endDoubleLoop;
      }
    }

endDoubleLoop:
  _communicator.min(_equiv);
  if (_mat_from_file)
    LibmeshPetscCallA(_communicator.get(), MatDestroy(&file_mat));
}

Real
IsMatrixSymmetric::getValue() const
{
  return _equiv;
}

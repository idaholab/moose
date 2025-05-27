//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MatrixEqualityCheck.h"

#include "MooseUtils.h"
#include "libmesh/petsc_matrix.h"

registerMooseObject("NavierStokesApp", MatrixEqualityCheck);

InputParameters
MatrixEqualityCheck::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Report whether two matrices are the same or not.");
  params.addRequiredParam<std::string>("mat1", "The petsc binary mat file containing matrix1");
  params.addRequiredParam<std::string>("mat2", "The petsc binary mat file containing matrix2");
  params.addParam<Real>(
      "equivalence_tol", 1e-8, "The relative tolerance for comparing equivalence");
  return params;
}

MatrixEqualityCheck::MatrixEqualityCheck(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _equiv_tol(getParam<Real>("equivalence_tol")),
    _mat1_name(getParam<std::string>("mat1")),
    _mat2_name(getParam<std::string>("mat2"))
{
}

void
MatrixEqualityCheck::execute()
{
  _equiv = true;

  auto mat1 = Moose::PetscSupport::createMatrixFromFile(_communicator, _petsc_mat1, _mat1_name);
  auto mat2 = Moose::PetscSupport::createMatrixFromFile(_communicator, _petsc_mat2, _mat2_name);

  if ((mat1->row_start() != mat2->row_start()) || (mat1->row_stop() != mat2->row_stop()) ||
      (mat1->col_start() != mat2->col_start()) || (mat1->col_stop() != mat2->col_stop()))
  {
    _equiv = false;
    return;
  }

  for (const auto i : make_range(mat1->row_start(), mat1->row_stop()))
    for (const auto j : make_range(mat1->col_start(), mat1->col_stop()))
    {
      const auto val1 = (*mat1)(i, j);
      const auto val2 = (*mat2)(i, j);
      if (!MooseUtils::relativeFuzzyEqual(val1, val2, _equiv_tol) &&
          !MooseUtils::absoluteFuzzyEqual(val1, val2, _equiv_tol))
      {
        _equiv = false;
        return;
      }
    }
}

void
MatrixEqualityCheck::finalize()
{
  _communicator.min(_equiv);

  if (_petsc_mat1)
    LibmeshPetscCall(MatDestroy(&_petsc_mat1));
  if (_petsc_mat2)
    LibmeshPetscCall(MatDestroy(&_petsc_mat2));
}

Real
MatrixEqualityCheck::getValue() const
{
  return _equiv;
}

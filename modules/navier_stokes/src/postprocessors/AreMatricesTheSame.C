//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AreMatricesTheSame.h"

#include "MooseUtils.h"
#include "libmesh/sparse_matrix.h"

registerMooseObject("NavierStokesApp", AreMatricesTheSame);

InputParameters
AreMatricesTheSame::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Report whether two matrices are the same or not.");
  params.addRequiredParam<std::string>("mat1", "The matlab like mat file containing matrix1");
  params.addRequiredParam<std::string>("mat2", "The matlab like mat file containing matrix2");
  params.addParam<Real>("equivalence_tol", 1e-8, "The relative tolerance for comparing symmetry");
  return params;
}

AreMatricesTheSame::AreMatricesTheSame(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _equiv_tol(getParam<Real>("equivalence_tol")),
    _mat1_name(getParam<std::string>("mat1")),
    _mat2_name(getParam<std::string>("mat2"))
{
}

void
AreMatricesTheSame::initialize()
{
  _mat1 = SparseMatrix<Number>::build(_communicator);
  _mat2 = SparseMatrix<Number>::build(_communicator);
  _mat1->read_matlab(_mat1_name);
  _mat2->read_matlab(_mat2_name);
}

void
AreMatricesTheSame::execute()
{
  _equiv = true;

  if ((_mat1->row_start() != _mat2->row_start()) || (_mat1->row_stop() != _mat2->row_stop()) ||
      (_mat1->col_start() != _mat2->col_start()) || (_mat1->col_stop() != _mat2->col_stop()))
  {
    _equiv = false;
    return;
  }

  for (const auto i : make_range(_mat1->row_start(), _mat1->row_stop()))
    for (const auto j : make_range(_mat1->col_start(), _mat1->col_stop()))
      if (!MooseUtils::relativeFuzzyEqual((*_mat1)(i, j), (*_mat2)(i, j), _equiv_tol))
      {
        _equiv = false;
        return;
      }
}

void
AreMatricesTheSame::finalize()
{
  _communicator.min(_equiv);
}

Real
AreMatricesTheSame::getValue() const
{
  return _equiv;
}

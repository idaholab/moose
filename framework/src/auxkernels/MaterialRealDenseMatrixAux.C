//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRealDenseMatrixAux.h"

template <>
InputParameters
validParams<MaterialRealDenseMatrixAux>()
{
  InputParameters params = validParams<MaterialAuxBase<>>();
  params.addParam<unsigned int>("row", 0, "The row component to consider for this kernel");
  params.addParam<unsigned int>("column", 0, "The column component to consider for this kernel");
  return params;
}

MaterialRealDenseMatrixAux::MaterialRealDenseMatrixAux(const InputParameters & parameters)
  : MaterialAuxBase<DenseMatrix<Real>>(parameters),
    _row(getParam<unsigned int>("row")),
    _col(getParam<unsigned int>("column"))
{
}

Real
MaterialRealDenseMatrixAux::getRealValue()
{
  return _prop[_qp](_row, _col);
}

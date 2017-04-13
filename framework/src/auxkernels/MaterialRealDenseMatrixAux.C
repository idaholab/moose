/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

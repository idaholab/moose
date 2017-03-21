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

#include "MaterialRealTensorValueAux.h"

template <>
InputParameters
validParams<MaterialRealTensorValueAux>()
{
  InputParameters params = validParams<MaterialAuxBase<>>();
  params.addParam<unsigned int>("row", 0, "The row component to consider for this kernel");
  params.addParam<unsigned int>("column", 0, "The column component to consider for this kernel");
  return params;
}

MaterialRealTensorValueAux::MaterialRealTensorValueAux(const InputParameters & parameters)
  : MaterialAuxBase<RealTensorValue>(parameters),
    _row(getParam<unsigned int>("row")),
    _col(getParam<unsigned int>("column"))
{
  if (_row > LIBMESH_DIM)
    mooseError(
        "The row component ", _row, " does not exist for ", LIBMESH_DIM, " dimensional problems");
  if (_col > LIBMESH_DIM)
    mooseError("The column component ",
               _col,
               " does not exist for ",
               LIBMESH_DIM,
               " dimensional problems");
}

Real
MaterialRealTensorValueAux::getRealValue()
{
  return _prop[_qp](_row, _col);
}

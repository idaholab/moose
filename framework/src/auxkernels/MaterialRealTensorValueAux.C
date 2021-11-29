//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRealTensorValueAux.h"

registerMooseObject("MooseApp", MaterialRealTensorValueAux);

InputParameters
MaterialRealTensorValueAux::validParams()
{
  InputParameters params = MaterialAuxBase<>::validParams();
  params.addClassDescription("Object for extracting a component of a rank two tensor material "
                             "property to populate an auxiliary variable.");
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

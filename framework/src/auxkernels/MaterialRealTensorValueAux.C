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
registerMooseObject("MooseApp", ADMaterialRealTensorValueAux);

template <bool is_ad>
InputParameters
MaterialRealTensorValueAuxTempl<is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<RealTensorValue, is_ad>::validParams();
  params.addClassDescription("Object for extracting a component of a rank two tensor material "
                             "property to populate an auxiliary variable.");
  params.addParam<unsigned int>("row", 0, "The row component to consider for this kernel");
  params.addParam<unsigned int>("column", 0, "The column component to consider for this kernel");
  return params;
}

template <bool is_ad>
MaterialRealTensorValueAuxTempl<is_ad>::MaterialRealTensorValueAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<RealTensorValue, is_ad>(parameters),
    _row(this->template getParam<unsigned int>("row")),
    _col(this->template getParam<unsigned int>("column"))
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

template <bool is_ad>
Real
MaterialRealTensorValueAuxTempl<is_ad>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_full_value(_row, _col));
}

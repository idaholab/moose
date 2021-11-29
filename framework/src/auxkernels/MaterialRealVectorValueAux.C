//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRealVectorValueAux.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", MaterialRealVectorValueAux);
registerMooseObject("MooseApp", ADMaterialRealVectorValueAux);

template <bool is_ad>
InputParameters
MaterialRealVectorValueAuxTempl<is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<RealVectorValue, is_ad>::validParams();
  params.addClassDescription(
      "Capture a component of a vector material property in an auxiliary variable.");
  params.addParam<unsigned int>("component", 0, "The vector component to consider for this kernel");

  return params;
}

template <bool is_ad>
MaterialRealVectorValueAuxTempl<is_ad>::MaterialRealVectorValueAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<RealVectorValue, is_ad>(parameters),
    _component(this->template getParam<unsigned int>("component"))
{
  if (_component > LIBMESH_DIM)
    this->mooseError(
        "The component ", _component, " does not exist for ", LIBMESH_DIM, " dimensional problems");
}

template <bool is_ad>
Real
MaterialRealVectorValueAuxTempl<is_ad>::getRealValue()
{
  return MetaPhysicL::raw_value(this->_prop[this->_qp](_component));
}

template class MaterialRealVectorValueAuxTempl<false>;
template class MaterialRealVectorValueAuxTempl<true>;

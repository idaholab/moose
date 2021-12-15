//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRealEigenVectorAux.h"

#include "metaphysicl/raw_type.h"

// We don't use AD with array variables yet, so the AD object is not instantiated.
// It can be easily done by registering a new object and adding a new typedef in the header.
registerMooseObject("MooseApp", MaterialRealEigenVectorAux);

template <bool is_ad>
InputParameters
MaterialRealEigenVectorAuxTempl<is_ad>::validParams()
{
  InputParameters params = MaterialAuxBaseTempl<RealEigenVector, is_ad>::validParams();
  params.addClassDescription(
      "Capture a component of an array material property in an auxiliary variable.");
  params.addParam<unsigned int>("component", 0, "The array component to consider for this kernel");
  return params;
}

template <bool is_ad>
MaterialRealEigenVectorAuxTempl<is_ad>::MaterialRealEigenVectorAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<RealEigenVector, is_ad>(parameters),
    _component(this->template getParam<unsigned int>("component"))
{
}

template <bool is_ad>
Real
MaterialRealEigenVectorAuxTempl<is_ad>::getRealValue()
{
  if (_component >= this->_prop[this->_qp].size())
    this->paramError("component",
                     "Requested component ",
                     _component,
                     " for array material property ",
                     this->template getParam<MaterialPropertyName>("property"),
                     ", which has ",
                     this->_prop[this->_qp].size(),
                     " component(s).");
  return MetaPhysicL::raw_value(this->_prop[this->_qp][_component]);
}

template class MaterialRealEigenVectorAuxTempl<false>;

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorMaterialRealEigenVectorAux.h"

#include "metaphysicl/raw_type.h"

// We don't use AD with array variables yet, so the AD object is not instantiated.
// It can be easily done by registering a new object and adding a new typedef in the header.
registerMooseObject("MooseApp", VectorMaterialRealEigenVectorAux);

template <bool is_ad>
InputParameters
VectorMaterialRealEigenVectorAuxTempl<is_ad>::validParams()
{
  InputParameters params =
      MaterialAuxBaseTempl<RealEigenVector, is_ad, RealEigenVector>::validParams();
  params.addClassDescription(
      "Converts an array-quantity material property into an array auxiliary variable");
  return params;
}

template <bool is_ad>
VectorMaterialRealEigenVectorAuxTempl<is_ad>::VectorMaterialRealEigenVectorAuxTempl(
    const InputParameters & parameters)
  : MaterialAuxBaseTempl<RealEigenVector, is_ad, RealEigenVector>(parameters)
{
}

template <bool is_ad>
RealEigenVector
VectorMaterialRealEigenVectorAuxTempl<is_ad>::getRealValue()
{
  if (this->_var.count() != this->_prop[this->_qp].size())
    this->paramError("variable",
                     "Variable ",
                     this->template getParam<AuxVariableName>("variable"),
                     " has ",
                     this->_var.count(),
                     " component(s) but the material ",
                     this->template getParam<MaterialPropertyName>("property"),
                     " has ",
                     this->_prop[this->_qp].size(),
                     " component(s)");
  return MetaPhysicL::raw_value(this->_prop[this->_qp]);
}

template class VectorMaterialRealEigenVectorAuxTempl<false>;

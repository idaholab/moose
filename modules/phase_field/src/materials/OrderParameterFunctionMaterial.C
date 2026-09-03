//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OrderParameterFunctionMaterial.h"

template <bool is_ad>
InputParameters
OrderParameterFunctionMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addCoupledVar("eta", "Order parameter variable");
  params.addParam<std::string>("function_name", "f", "actual name for f(eta), i.e. 'h' or 'g'");
  return params;
}

template <bool is_ad>
OrderParameterFunctionMaterialTempl<is_ad>::OrderParameterFunctionMaterialTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _eta(this->template coupledGenericValue<is_ad>("eta")),
    _eta_var(this->coupled("eta")),
    _eta_name(this->coupledName("eta", 0)),
    _function_name(this->template getParam<std::string>("function_name")),
    _prop_f(this->template declareGenericProperty<Real, is_ad>(_function_name)),
    _prop_df(this->template declarePropertyDerivative<Real, is_ad>(_function_name, _eta_name)),
    _prop_d2f(
        this->template declarePropertyDerivative<Real, is_ad>(_function_name, _eta_name, _eta_name))
{
}

template class OrderParameterFunctionMaterialTempl<false>;
template class OrderParameterFunctionMaterialTempl<true>;

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OrderParameterFunctionMaterial.h"

InputParameters
OrderParameterFunctionMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addCoupledVar("eta", "Order parameter variable");
  params.addParam<std::string>("function_name", "f", "actual name for f(eta), i.e. 'h' or 'g'");
  return params;
}

OrderParameterFunctionMaterial::OrderParameterFunctionMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _eta(coupledValue("eta")),
    _eta_var(coupled("eta")),
    _eta_name(coupledName("eta", 0)),
    _function_name(getParam<std::string>("function_name")),
    _prop_f(declareProperty<Real>(_function_name)),
    _prop_df(declarePropertyDerivative<Real>(_function_name, _eta_name)),
    _prop_d2f(declarePropertyDerivative<Real>(_function_name, _eta_name, _eta_name))
{
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantMaterial.h"

registerMooseObject("ThermalHydraulicsApp", ConstantMaterial);

InputParameters
ConstantMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<Real>("value", 0., "Constant value being assigned into the property");
  params.addRequiredParam<std::string>("property_name", "The property name to declare");
  params.addCoupledVar(
      "derivative_vars",
      "Names of variables for which to create (zero) material derivative properties");
  return params;
}

ConstantMaterial::ConstantMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _value(getParam<Real>("value")),
    _property_name(getParam<std::string>("property_name")),
    _property(declareProperty<Real>(_property_name)),
    _n_derivative_vars(coupledComponents("derivative_vars"))
{
  // get references to new material property derivatives
  _derivative_properties.resize(_n_derivative_vars);
  if (!isCoupledConstant("derivative_vars"))
    for (unsigned int i = 0; i < _n_derivative_vars; ++i)
      _derivative_properties[i] =
          &declarePropertyDerivative<Real>(_property_name, coupledName("derivative_vars", i));
}

void
ConstantMaterial::computeQpProperties()
{
  _property[_qp] = _value;

  for (unsigned int i = 0; i < _n_derivative_vars; ++i)
    if (_derivative_properties[i])
      (*_derivative_properties[i])[_qp] = 0;
}

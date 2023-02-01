//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearTestMaterial.h"

registerMooseObject("ThermalHydraulicsTestApp", LinearTestMaterial);

InputParameters
LinearTestMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredCoupledVar("vars",
                               "List of aux variables that the material property depends upon");
  params.addRequiredParam<std::vector<Real>>(
      "slopes", "Slopes of the material property with respect to each aux variable");
  params.addParam<Real>("shift", 0, "Shift constant: 'b' in 'y = m * x + b'");
  params.addRequiredParam<MaterialPropertyName>("name", "Name of the new material property");

  return params;
}

LinearTestMaterial::LinearTestMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _n_vars(coupledComponents("vars")),
    _slopes(getParam<std::vector<Real>>("slopes")),
    _shift(getParam<Real>("shift")),
    _y_name(getParam<MaterialPropertyName>("name")),
    _y(declareProperty<Real>(_y_name))
{
  // check that number of provided slopes is consistent with number of variables
  if (_slopes.size() != _n_vars)
    mooseError(
        "LinearTestMaterial:", name(), ": Parameters 'vars' and 'slopes' must have same size.");

  // get references to coupled variables and new material property derivatives
  _vars.resize(_n_vars);
  _y_derivatives.resize(_n_vars);
  for (unsigned int i = 0; i < _n_vars; ++i)
  {
    _vars[i] = &coupledValue("vars", i);
    if (!isCoupledConstant("vars"))
      _y_derivatives[i] = &declarePropertyDerivative<Real>(_y_name, coupledName("vars", i));
  }
}

void
LinearTestMaterial::computeQpProperties()
{
  _y[_qp] = _shift;
  for (unsigned int i = 0; i < _n_vars; ++i)
  {
    _y[_qp] += _slopes[i] * (*_vars[i])[_qp];
    if (_y_derivatives[i])
      (*_y_derivatives[i])[_qp] = _slopes[i];
  }
}

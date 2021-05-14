//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SwitchingFunctionMaterial.h"

registerMooseObject("PhaseFieldApp", SwitchingFunctionMaterial);

InputParameters
SwitchingFunctionMaterial::validParams()
{
  InputParameters params = OrderParameterFunctionMaterial::validParams();
  params.addClassDescription(
      "Helper material to provide $h(\\eta)$ and its derivative in one of two "
      "polynomial forms.\nSIMPLE: $3\\eta^2-2\\eta^3$\nHIGH: "
      "$\\eta^3(6\\eta^2-15\\eta+10)$");
  MooseEnum h_order("SIMPLE=0 HIGH", "SIMPLE");
  params.addParam<MooseEnum>(
      "h_order", h_order, "Polynomial order of the switching function h(eta)");
  params.set<std::string>("function_name") = std::string("h");
  return params;
}

SwitchingFunctionMaterial::SwitchingFunctionMaterial(const InputParameters & parameters)
  : OrderParameterFunctionMaterial(parameters), _h_order(getParam<MooseEnum>("h_order"))
{
}

void
SwitchingFunctionMaterial::computeQpProperties()
{
  Real n = _eta[_qp];
  n = n > 1 ? 1 : (n < 0 ? 0 : n);

  switch (_h_order)
  {
    case 0: // SIMPLE
      _prop_f[_qp] = 3.0 * n * n - 2.0 * n * n * n;
      _prop_df[_qp] = 6.0 * n - 6.0 * n * n;
      _prop_d2f[_qp] = 6.0 - 12.0 * n;
      break;

    case 1: // HIGH
      _prop_f[_qp] = n * n * n * (6.0 * n * n - 15.0 * n + 10.0);
      _prop_df[_qp] = 30.0 * n * n * (n * n - 2.0 * n + 1.0);
      _prop_d2f[_qp] = n * (120.0 * n * n - 180.0 * n + 60.0);
      break;

    default:
      mooseError("Internal error");
  }
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MixedSwitchingFunctionMaterial.h"

registerMooseObject("PhaseFieldApp", MixedSwitchingFunctionMaterial);

InputParameters
MixedSwitchingFunctionMaterial::validParams()
{
  InputParameters params = OrderParameterFunctionMaterial::validParams();
  params.addClassDescription("Helper material to provide h(eta) and its derivative in one of two "
                             "polynomial forms. MIX234 and MIX246");
  MooseEnum h_order("MIX234=0 MIX246", "MIX234");
  params.addParam<MooseEnum>(
      "h_order", h_order, "Polynomial order of the switching function h(eta)");
  params.set<std::string>("function_name") = std::string("h");

  params.addRangeCheckedParam<Real>(
      "weight", 1.0, "weight <= 1 & weight >= 0", "Weight parameter for MIX type h(eta)");

  return params;
}

MixedSwitchingFunctionMaterial::MixedSwitchingFunctionMaterial(const InputParameters & parameters)
  : OrderParameterFunctionMaterial(parameters),
    _h_order(getParam<MooseEnum>("h_order")),
    _weight(getParam<Real>("weight"))
{
}

void
MixedSwitchingFunctionMaterial::computeQpProperties()
{
  Real n = _eta[_qp];
  n = n > 1 ? 1 : (n < 0 ? 0 : n);

  switch (_h_order)
  {
    case 0: // MIX234
      _prop_f[_qp] =
          n * n * (3.0 * _weight + (4.0 - 6.0 * _weight) * n + (3.0 * _weight - 3.0) * n * n);
      _prop_df[_qp] =
          n * (6.0 * _weight + (12.0 - 18.0 * _weight) * n + (12.0 * _weight - 12.0) * n * n);
      _prop_d2f[_qp] = 6.0 * _weight + ((24.0 - 36.0 * _weight) + (36.0 * _weight - 36.0) * n) * n;
      break;

    case 1: // MIX246
      _prop_f[_qp] =
          n * n * (2.0 * _weight + n * n * ((3.0 - 4.0 * _weight) + (2.0 * _weight - 2.0) * n * n));
      _prop_df[_qp] =
          n * (4.0 * _weight + n * n * ((12.0 - 16.0 * _weight) + (12.0 * _weight - 12.0) * n * n));
      _prop_d2f[_qp] =
          4.0 * _weight + n * n * ((36.0 - 48.0 * _weight) + (60.0 * _weight - 60.0) * n * n);
      break;

    default:
      mooseError("Internal error");
  }
}

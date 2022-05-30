//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectrostaticContactCondition.h"

registerMooseObject("ElectromagneticsApp", ElectrostaticContactCondition);

InputParameters
ElectrostaticContactCondition::validParams()
{
  InputParameters params = ADInterfaceKernel::validParams();
  params.addParam<MaterialPropertyName>(
      "primary_conductivity", "electrical_conductivity", "Conductivity on the primary block.");
  params.addParam<MaterialPropertyName>(
      "secondary_conductivity", "electrical_conductivity", "Conductivity on the secondary block.");
  params.addParam<MaterialPropertyName>(
      "mean_hardness",
      "mean_hardness",
      "Geometric mean of the hardness of each contacting material.");
  params.addParam<Real>("user_electrical_contact_conductance",
                        "User-supplied electrical contact conductance coefficient.");
  params.addParam<FunctionName>("mechanical_pressure",
                                0.0,
                                "Mechanical pressure uniformly applied at the contact surface area "
                                "(Pressure = Force / Surface Area).");
  params.addClassDescription(
      "Interface condition that describes the current continuity and contact conductance across a "
      "boundary formed between two dissimilar materials (resulting in a potential discontinuity). "
      "Conductivity on each side of the boundary is defined via the material properties system.");
  return params;
}

ElectrostaticContactCondition::ElectrostaticContactCondition(const InputParameters & parameters)
  : ADInterfaceKernel(parameters),
    _conductivity_primary(getADMaterialProperty<Real>("primary_conductivity")),
    _conductivity_secondary(getNeighborADMaterialProperty<Real>("secondary_conductivity")),
    _mean_hardness(isParamValid("user_electrical_contact_conductance")
                       ? getGenericZeroMaterialProperty<Real, true>("mean_hardness")
                       : getADMaterialProperty<Real>("mean_hardness")),
    _mechanical_pressure(getFunction("mechanical_pressure")),
    _user_contact_conductance(isParamValid("user_electrical_contact_conductance")
                                  ? getParam<Real>("user_electrical_contact_conductance")
                                  : _real_zero),
    _alpha_electric(64.0),
    _beta_electric(0.35)
{
  _conductance_was_set = parameters.isParamSetByUser("user_electrical_contact_conductance");
  _mean_hardness_was_set = parameters.isParamSetByUser("mean_hardness");

  if (_conductance_was_set && _mean_hardness_was_set)
    mooseError(
        "In ",
        _name,
        ", both user-supplied electrical contact conductance and mean hardness values (for "
        "calculating contact conductance) have been provided. Please only provide one or the "
        "other!");
}

ADReal
ElectrostaticContactCondition::computeQpResidual(Moose::DGResidualType type)
{
  ADReal res = 0.0;
  ADReal contact_conductance = 0.0;

  ADReal mean_conductivity = 2 * _conductivity_primary[_qp] * _conductivity_secondary[_qp] /
                             (_conductivity_primary[_qp] + _conductivity_secondary[_qp]);

  if (_conductance_was_set && !_mean_hardness_was_set)
    contact_conductance = _user_contact_conductance;
  else if (_mean_hardness_was_set && !_conductance_was_set)
    contact_conductance =
        _alpha_electric * mean_conductivity *
        std::pow((_mechanical_pressure.value(_t, _q_point[_qp]) / _mean_hardness[_qp]),
                 _beta_electric);

  switch (type)
  {
    case Moose::Element:
      res = -contact_conductance * (_neighbor_value[_qp] - _u[_qp]) * _test[_i][_qp];
      break;

    case Moose::Neighbor:
      res = _conductivity_primary[_qp] * _grad_u[_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
      break;
  }

  return res;
}

#include "ElectrostaticContactCondition.h"

registerMooseObject("ElkApp", ElectrostaticContactCondition);

InputParameters
ElectrostaticContactCondition::validParams()
{
  InputParameters params = ADInterfaceKernel::validParams();
  params.addParam<MaterialPropertyName>(
      "master_conductivity", "electrical_conductivity", "Conductivity on the master block.");
  params.addParam<MaterialPropertyName>(
      "neighbor_conductivity", "electrical_conductivity", "Conductivity on the neighbor block.");
  params.addParam<MaterialPropertyName>(
      "mean_hardness",
      "mean_hardness",
      "Geometric mean of the hardness of each contacting material.");
  params.addParam<Real>("user_electrical_contact_resistance",
                        "User-supplied electrical contact resistance coefficient.");
  params.addParam<Real>("mechanical_pressure",
                        "Mechanical pressure uniformly applied at the contact surface area "
                        "(Pressure = Force / Surface Area).");
  params.addClassDescription(
      "Interface condition that describes the current continuity and contact resistance across a "
      "boundary formed between two dissimilar materials (resulting in a potential discontinuity). "
      "Conductivity on each side of the boundary is defined via the material peoperties system.");
  return params;
}

ElectrostaticContactCondition::ElectrostaticContactCondition(const InputParameters & parameters)
  : ADInterfaceKernel(parameters),
    _conductivity_master(getMaterialProperty<Real>("master_conductivity")),
    _conductivity_neighbor(getNeighborMaterialProperty<Real>("neighbor_conductivity")),
    _mean_hardness(isParamValid("user_electrical_contact_resistance")
                       ? getGenericZeroMaterialProperty<Real, true>("mean_hardness")
                       : getADMaterialProperty<Real>("mean_hardness")),
    _mechanical_pressure(isParamValid("mechanical_pressure") ? getParam<Real>("mechanical_pressure")
                                                             : _real_zero),
    _user_contact_resistance(isParamValid("user_electrical_contact_resistance")
                                 ? getParam<Real>("user_electrical_contact_resistance")
                                 : _real_zero),
    _alpha_electric(64.0),
    _beta_electric(0.35)
{
  _resistance_was_set = parameters.isParamSetByUser("user_electrical_contact_resistance");
  _mean_hardness_was_set = parameters.isParamSetByUser("mean_hardness");
}

ADReal
ElectrostaticContactCondition::computeQpResidual(Moose::DGResidualType type)
{
  ADReal res = 0.0;
  ADReal contact_resistance = 0.0;

  ADReal mean_conductivity = 2 * _conductivity_master[_qp] * _conductivity_neighbor[_qp] /
                             (_conductivity_master[_qp] + _conductivity_neighbor[_qp]);

  if (_resistance_was_set && !_mean_hardness_was_set)
  {
    contact_resistance = _user_contact_resistance;
  }
  else if (_mean_hardness_was_set && !_resistance_was_set)
  {
    contact_resistance = _alpha_electric * mean_conductivity *
                         std::pow((_mechanical_pressure / _mean_hardness[_qp]), _beta_electric);
  }
  else
  {
    mooseError("In ",
               _name,
               ", both user-supplied electrical contact resistance and mean hardness values (for "
               "calculating contact resistance) have been provided. Please only provide one or the "
               "other!");
  }

  switch (type)
  {
    case Moose::Element:
      res = 0.5 *
            (_conductivity_neighbor[_qp] * _grad_neighbor_value[_qp] * _normals[_qp] -
             contact_resistance * (_neighbor_value[_qp] - _u[_qp])) *
            _test[_i][_qp];
      break;

    case Moose::Neighbor:
      res = _conductivity_master[_qp] * _grad_u[_qp] * _normals[_qp] * _test_neighbor[_i][_qp];
      break;
  }

  return res;
}

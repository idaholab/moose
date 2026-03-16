//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVEnthalpyVolumetricHeatTransfer.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", LinearFVEnthalpyVolumetricHeatTransfer);

InputParameters
LinearFVEnthalpyVolumetricHeatTransfer::validParams()
{
  InputParameters params = LinearFVVolumetricHeatTransfer::validParams();
  params.addClassDescription(
      "Represents a volumetric heat transfer term between two porous-medium energy equations when "
      "the solved variable is specific enthalpy and the current-side temperature is linearized "
      "with dT/dh ~= 1/cp.");
  params.addRequiredParam<MooseFunctorName>(
      NS::cp,
      "Specific heat used to linearize the current-side temperature from the enthalpy variable.");
  return params;
}

LinearFVEnthalpyVolumetricHeatTransfer::LinearFVEnthalpyVolumetricHeatTransfer(
    const InputParameters & params)
  : LinearFVVolumetricHeatTransfer(params), _cp(getFunctor<Real>(NS::cp))
{
}

Real
LinearFVEnthalpyVolumetricHeatTransfer::computeMatrixContribution()
{
  const auto elem = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();
  const auto cp = _cp(elem, state);

  if (cp <= 0.0)
    mooseError(name(), ": cp must stay positive when solving for enthalpy.");

  return _h_solid_fluid(elem, state) * _current_elem_volume / cp;
}

Real
LinearFVEnthalpyVolumetricHeatTransfer::computeRightHandSideContribution()
{
  const auto elem = makeElemArg(_current_elem_info->elem());
  const auto state = determineState();
  const auto cp = _cp(elem, state);

  if (cp <= 0.0)
    mooseError(name(), ": cp must stay positive when solving for enthalpy.");

  const auto & current_temperature = _is_solid ? _temp_solid : _temp_fluid;
  const auto & other_temperature = _is_solid ? _temp_fluid : _temp_solid;
  const auto h_current = _var.getElemValue(*_current_elem_info, state);
  const auto linearized_current_temperature = h_current / cp;

  return _h_solid_fluid(elem, state) *
         (other_temperature.getElemValue(*_current_elem_info, state) -
          current_temperature.getElemValue(*_current_elem_info, state) +
          linearized_current_temperature) *
         _current_elem_volume;
}

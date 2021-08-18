//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NS.h"
#include "PNSInitialCondition.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("NavierStokesApp", PNSInitialCondition);

InputParameters
PNSInitialCondition::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription(
      "PNSInitialCondition sets intial constant values for any porous flow variable.");

  MooseEnum variable_types(
      MooseUtils::join(std::vector<std::string>{NS::specific_total_enthalpy,
                                                NS::specific_internal_energy,
                                                NS::mach_number,
                                                NS::pressure,
                                                NS::density,
                                                NS::momentum_x,
                                                NS::momentum_y,
                                                NS::momentum_z,
                                                NS::total_energy_density,
                                                NS::specific_volume,
                                                NS::temperature,
                                                NS::velocity_x,
                                                NS::velocity_y,
                                                NS::velocity_z,
                                                NS::superficial_velocity_x,
                                                NS::superficial_velocity_y,
                                                NS::superficial_velocity_z,
                                                NS::superficial_density,
                                                NS::superficial_momentum_x,
                                                NS::superficial_momentum_y,
                                                NS::superficial_momentum_z,
                                                NS::superficial_total_energy_density,
                                                NS::superficial_total_enthalpy_density},
                       " "));
  params.addParam<MooseEnum>(
      "variable_type",
      variable_types,
      "Specifies what this variable is in the Navier Stokes namespace of variables");
  params.addRequiredParam<Real>("initial_pressure",
                                "The initial pressure, assumed constant everywhere");
  params.addRequiredParam<Real>("initial_temperature",
                                "The initial temperature, assumed constant everywhere");
  params.addParam<RealVectorValue>(
      "initial_interstitial_velocity",
      "The initial interstitial velocity, assumed constant everywhere");
  params.addParam<RealVectorValue>("initial_superficial_velocity",
                                   "The initial superficial velocity, assumed constant everywhere");
  params.addCoupledVar("porosity", "Porosity variable (defaults to porosity material property).");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

PNSInitialCondition::PNSInitialCondition(const InputParameters & parameters)
  : InitialCondition(parameters),
    _variable_type(isParamValid("variable_type") ? getParam<MooseEnum>("variable_type")
                                                 : MooseEnum(_var.name(), _var.name())),
    _initial_pressure(getParam<Real>("initial_pressure")),
    _initial_temperature(getParam<Real>("initial_temperature")),
    _superficial_velocities_set(isParamValid("initial_superficial_velocity") ? true : false),
    _eps(isCoupled("porosity") ? coupledValue("porosity")
                               : getMaterialProperty<Real>(NS::porosity).get()),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
  if (isParamValid("initial_superficial_velocity"))
    _initial_superficial_velocity = getParam<RealVectorValue>("initial_superficial_velocity");
  if (isParamValid("initial_interstitial_velocity"))
    _initial_interstitial_velocity = getParam<RealVectorValue>("initial_interstitial_velocity");
  if (isParamValid("initial_superficial_velocity") && isParamValid("initial_interstitial_velocity"))
    paramError("Either superficial or interstitial velocities may be specified.");
}

Real
PNSInitialCondition::value(const Point & /*p*/)
{
  // Compute velocities
  if (_superficial_velocities_set)
    _initial_interstitial_velocity = _initial_superficial_velocity / _eps[_qp];
  else
    _initial_superficial_velocity = _initial_interstitial_velocity * _eps[_qp];

  const Real rho_initial = _fp.rho_from_p_T(_initial_pressure, _initial_temperature);

  // TODO: The internal energy could be computed by the IdealGasFluidProperties.
  const Real e_initial = _fp.cv() * _initial_temperature;
  const Real et_initial = e_initial + 0.5 * _initial_interstitial_velocity.norm_sq();
  const Real v_initial = 1. / rho_initial;

  if (_variable_type == NS::specific_total_enthalpy)
    return et_initial + _initial_pressure / rho_initial;

  if (_variable_type == NS::superficial_total_enthalpy_density)
    return (et_initial + _initial_pressure / rho_initial) * _eps[_qp];

  if (_variable_type == NS::specific_internal_energy)
    return e_initial;

  if (_variable_type == NS::mach_number)
    return _initial_superficial_velocity.norm() / _fp.c_from_v_e(v_initial, e_initial);

  if (_variable_type == NS::pressure)
    return _initial_pressure;

  if (_variable_type == NS::density)
    return rho_initial;

  if (_variable_type == NS::superficial_density)
    return rho_initial * _eps[_qp];

  if (_variable_type == NS::superficial_momentum_x)
    return rho_initial * _initial_superficial_velocity(0);

  if (_variable_type == NS::superficial_momentum_y)
    return rho_initial * _initial_superficial_velocity(1);

  if (_variable_type == NS::superficial_momentum_z)
    return rho_initial * _initial_superficial_velocity(2);

  if (_variable_type == NS::momentum_x)
    return rho_initial * _initial_interstitial_velocity(0);

  if (_variable_type == NS::momentum_y)
    return rho_initial * _initial_interstitial_velocity(1);

  if (_variable_type == NS::momentum_z)
    return rho_initial * _initial_interstitial_velocity(2);

  if (_variable_type == NS::total_energy_density)
    return rho_initial * et_initial;

  if (_variable_type == NS::superficial_total_energy_density)
    return rho_initial * et_initial * _eps[_qp];

  if (_variable_type == NS::specific_volume)
    return v_initial;

  if (_variable_type == NS::temperature)
    return _initial_temperature;

  if (_variable_type == NS::superficial_velocity_x)
    return _initial_superficial_velocity(0);

  if (_variable_type == NS::superficial_velocity_y)
    return _initial_superficial_velocity(1);

  if (_variable_type == NS::superficial_velocity_z)
    return _initial_superficial_velocity(2);

  if (_variable_type == NS::velocity_x)
    return _initial_interstitial_velocity(0);

  if (_variable_type == NS::velocity_y)
    return _initial_interstitial_velocity(1);

  if (_variable_type == NS::velocity_z)
    return _initial_interstitial_velocity(2);

  // If we got here, then the variable name was not one of the ones we know about.
  mooseError("Unrecognized variable: ", _variable_type);
  return 0.;
}

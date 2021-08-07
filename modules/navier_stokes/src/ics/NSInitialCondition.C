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
#include "NSInitialCondition.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("NavierStokesApp", NSInitialCondition);

InputParameters
NSInitialCondition::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription("NSInitialCondition sets intial constant values for all variables.");
  params.addDeprecatedParam<std::string>("pressure_variable_name",
                                         NS::pressure,
                                         "The name of the pressure variable",
                                         "pressure_variable_name is deprecated, use variable_type");
  MooseEnum variable_types(MooseUtils::join(std::vector<std::string>{NS::specific_total_enthalpy,
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
                                                                     NS::velocity_z},
                                            " "));
  params.addParam<MooseEnum>(
      "variable_type",
      variable_types,
      "Specifies what this variable is in the Navier Stokes namespace of variables");
  params.addRequiredParam<Real>("initial_pressure",
                                "The initial pressure, assumed constant everywhere");
  params.addRequiredParam<Real>("initial_temperature",
                                "The initial temperature, assumed constant everywhere");
  params.addRequiredParam<RealVectorValue>("initial_velocity",
                                           "The initial velocity, assumed constant everywhere");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

NSInitialCondition::NSInitialCondition(const InputParameters & parameters)
  : InitialCondition(parameters),
    _variable_type(isParamValid("variable_type") ? getParam<MooseEnum>("variable_type")
                                                 : MooseEnum(_var.name(), _var.name())),
    _initial_pressure(getParam<Real>("initial_pressure")),
    _initial_temperature(getParam<Real>("initial_temperature")),
    _initial_velocity(getParam<RealVectorValue>("initial_velocity")),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties")),
    _pressure_variable_name(getParam<std::string>("pressure_variable_name"))
{
}

Real
NSInitialCondition::value(const Point & /*p*/)
{
  const Real rho_initial = _fp.rho_from_p_T(_initial_pressure, _initial_temperature);

  // TODO: The internal energy could be computed by the IdealGasFluidProperties.
  const Real e_initial = _fp.cv() * _initial_temperature;
  const Real et_initial = e_initial + 0.5 * _initial_velocity.norm_sq();
  const Real v_initial = 1. / rho_initial;

  if (_variable_type == NS::specific_total_enthalpy)
    return et_initial + _initial_pressure / rho_initial;

  if (_variable_type == NS::specific_internal_energy)
    return e_initial;

  if (_variable_type == NS::mach_number)
    return _initial_velocity.norm() / _fp.c_from_v_e(v_initial, e_initial);

  if (_variable_type == NS::pressure || _variable_type == _pressure_variable_name)
    return _initial_pressure;

  if (_variable_type == NS::density)
    return rho_initial;

  if (_variable_type == NS::momentum_x)
    return rho_initial * _initial_velocity(0);

  if (_variable_type == NS::momentum_y)
    return rho_initial * _initial_velocity(1);

  if (_variable_type == NS::momentum_z)
    return rho_initial * _initial_velocity(2);

  if (_variable_type == NS::total_energy_density)
    return rho_initial * et_initial;

  if (_variable_type == NS::specific_volume)
    return v_initial;

  if (_variable_type == NS::temperature)
    return _initial_temperature;

  if (_variable_type == NS::velocity_x)
    return _initial_velocity(0);

  if (_variable_type == NS::velocity_y)
    return _initial_velocity(1);

  if (_variable_type == NS::velocity_z)
    return _initial_velocity(2);

  // If we got here, then the variable name was not one of the ones we know about.
  mooseError("Unrecognized variable: ", _variable_type);
  return 0.;
}

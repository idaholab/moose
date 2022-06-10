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
#include "NSFunctionInitialCondition.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

// MOOSE includes
#include "MooseVariable.h"

registerMooseObject("NavierStokesApp", NSFunctionInitialCondition);

InputParameters
NSFunctionInitialCondition::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription("Sets intial values for all variables.");
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
  params.addRequiredParam<FunctionName>("initial_pressure", "The initial pressure");
  params.addRequiredParam<FunctionName>("initial_temperature", "The initial temperature");
  params.addRequiredParam<std::vector<FunctionName>>("initial_velocity", "The initial velocity");
  params.addRequiredParam<UserObjectName>("fluid_properties",
                                          "The name of the user object for fluid properties");

  return params;
}

NSFunctionInitialCondition::NSFunctionInitialCondition(const InputParameters & parameters)
  : InitialCondition(parameters),
    _variable_type(isParamValid("variable_type") ? getParam<MooseEnum>("variable_type")
                                                 : MooseEnum(_var.name(), _var.name())),
    _initial_pressure(getFunction("initial_pressure")),
    _initial_temperature(getFunction("initial_temperature")),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties")),
    _pressure_variable_name(getParam<std::string>("pressure_variable_name"))
{
  for (const auto i : make_range(Moose::dim))
    _initial_velocity.push_back(
        &getFunctionByName(getParam<std::vector<FunctionName>>("initial_velocity")[i]));
}

Real
NSFunctionInitialCondition::value(const Point & p)
{
  const Real initial_pressure = _initial_pressure.value(_t, p);
  const Real initial_temperature = _initial_temperature.value(_t, p);
  const RealVectorValue initial_velocity = {_initial_velocity[0]->value(_t, p),
                                            _initial_velocity[1]->value(_t, p),
                                            _initial_velocity[2]->value(_t, p)};

  const Real rho_initial = _fp.rho_from_p_T(initial_pressure, initial_temperature);

  // TODO: The internal energy could be computed by the IdealGasFluidProperties.
  const Real e_initial = _fp.cv() * initial_temperature;
  const Real et_initial = e_initial + 0.5 * initial_velocity.norm_sq();
  const Real v_initial = 1. / rho_initial;

  if (_variable_type == NS::specific_total_enthalpy)
    return et_initial + initial_pressure / rho_initial;

  if (_variable_type == NS::specific_internal_energy)
    return e_initial;

  if (_variable_type == NS::mach_number)
    return initial_velocity.norm() / _fp.c_from_v_e(v_initial, e_initial);

  if (_variable_type == NS::pressure || _variable_type == _pressure_variable_name)
    return initial_pressure;

  if (_variable_type == NS::density)
    return rho_initial;

  if (_variable_type == NS::momentum_x)
    return rho_initial * initial_velocity(0);

  if (_variable_type == NS::momentum_y)
    return rho_initial * initial_velocity(1);

  if (_variable_type == NS::momentum_z)
    return rho_initial * initial_velocity(2);

  if (_variable_type == NS::total_energy_density)
    return rho_initial * et_initial;

  if (_variable_type == NS::specific_volume)
    return v_initial;

  if (_variable_type == NS::temperature)
    return initial_temperature;

  if (_variable_type == NS::velocity_x)
    return initial_velocity(0);

  if (_variable_type == NS::velocity_y)
    return initial_velocity(1);

  if (_variable_type == NS::velocity_z)
    return initial_velocity(2);

  // If we got here, then the variable name was not one of the ones we know about.
  mooseError("Unrecognized variable: ", _variable_type);
  return 0.;
}

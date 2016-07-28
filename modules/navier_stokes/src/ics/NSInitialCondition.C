/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "NSInitialCondition.h"

// FluidProperties includes
#include "IdealGasFluidProperties.h"

template<>
InputParameters validParams<NSInitialCondition>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("initial_pressure", "The initial pressure, assumed constant everywhere");
  params.addRequiredParam<Real>("initial_temperature", "The initial temperature, assumed constant everywhere");
  params.addRequiredParam<RealVectorValue>("initial_velocity", "The initial velocity, assumed constant everywhere");
  params.addRequiredParam<UserObjectName>("fluid_properties", "The name of the user object for fluid properties");

  return params;
}

NSInitialCondition::NSInitialCondition(const InputParameters & parameters) :
    InitialCondition(parameters),
    _initial_pressure(getParam<Real>("initial_pressure")),
    _initial_temperature(getParam<Real>("initial_temperature")),
    _initial_velocity(getParam<RealVectorValue>("initial_velocity")),
    _fp(getUserObject<IdealGasFluidProperties>("fluid_properties"))
{
}

Real
NSInitialCondition::value(const Point & /*p*/)
{
  const Real rho_initial = _fp.rho(_initial_pressure, _initial_temperature);

  // TODO: The internal energy could be computed by the IdealGasFluidProperties.
  const Real e_initial = _fp.cv() * _initial_temperature;
  const Real E_initial = e_initial + 0.5 * _initial_velocity.norm_sq();
  const Real v_initial = 1. / rho_initial;

  if (_var.name() == "enthalpy")
    return E_initial + _initial_pressure / rho_initial;

  if (_var.name() == "internal_energy")
    return e_initial;

  if (_var.name() == "Mach")
    return _initial_velocity.norm() / _fp.c(v_initial, e_initial);

  if (_var.name() == "pressure")
    return _initial_pressure;

  if (_var.name() == "rho")
    return rho_initial;

  if (_var.name() == "rhou")
    return rho_initial * _initial_velocity(0);

  if (_var.name() == "rhov")
    return rho_initial * _initial_velocity(1);

  if (_var.name() == "rhow")
    return rho_initial * _initial_velocity(2);

  if (_var.name() == "rhoE")
    return rho_initial * E_initial;

  if (_var.name() == "specific_volume")
    return v_initial;

  if (_var.name() == "temperature")
    return _initial_temperature;

  if (_var.name() == "vel_x")
    return _initial_velocity(0);

  if (_var.name() == "vel_y")
    return _initial_velocity(1);

  if (_var.name() == "vel_z")
    return _initial_velocity(2);

  // If we got here, then the variable name was not one of the ones we know about.
  mooseError("Unrecognized variable: " << _var.name());
  return 0.;
}

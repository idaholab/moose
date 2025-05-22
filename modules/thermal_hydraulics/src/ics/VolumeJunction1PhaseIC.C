//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VolumeJunction1PhaseIC.h"
#include "Function.h"
#include "SinglePhaseFluidProperties.h"

registerMooseObject("ThermalHydraulicsApp", VolumeJunction1PhaseIC);

InputParameters
VolumeJunction1PhaseIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  MooseEnum quantity("rhoV rhouV rhovV rhowV rhoEV p T vel");
  params.addRequiredParam<MooseEnum>("quantity", quantity, "Which quantity to compute");
  params.addRequiredParam<FunctionName>("initial_p", "Initial pressure [Pa]");
  params.addRequiredParam<FunctionName>("initial_T", "Initial temperature [K]");
  params.addRequiredParam<FunctionName>("initial_vel_x", "Initial velocity in x-direction [m/s]");
  params.addRequiredParam<FunctionName>("initial_vel_y", "Initial velocity in y-direction [m/s]");
  params.addRequiredParam<FunctionName>("initial_vel_z", "Initial velocity in z-direction [m/s]");
  params.addRequiredParam<Real>("volume", "Volume of the junction [m^3]");
  params.addRequiredParam<Point>("position", "Spatial position of the center of the junction [m]");
  params.addRequiredParam<UserObjectName>("fluid_properties", "SinglePhaseFluidProperties object");
  params.addClassDescription("IC for junction variables in VolumeJunction1Phase.");
  return params;
}

VolumeJunction1PhaseIC::VolumeJunction1PhaseIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _quantity(getParam<MooseEnum>("quantity").getEnum<Quantity>()),
    _p_fn(getFunction("initial_p")),
    _T_fn(getFunction("initial_T")),
    _vel_x_fn(getFunction("initial_vel_x")),
    _vel_y_fn(getFunction("initial_vel_y")),
    _vel_z_fn(getFunction("initial_vel_z")),
    _volume(getParam<Real>("volume")),
    _position(getParam<Point>("position")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fluid_properties"))
{
}

Real
VolumeJunction1PhaseIC::value(const Point & /*p*/)
{
  const Real p = _p_fn.value(0, _position);
  const Real T = _T_fn.value(0, _position);
  const Real vel_x = _vel_x_fn.value(0, _position);
  const Real vel_y = _vel_y_fn.value(0, _position);
  const Real vel_z = _vel_z_fn.value(0, _position);

  const Real rho = _fp.rho_from_p_T(p, T);
  const RealVectorValue vel(vel_x, vel_y, vel_z);
  const Real E = _fp.e_from_p_rho(p, rho) + 0.5 * vel * vel;

  switch (_quantity)
  {
    case Quantity::RHOV:
      return rho * _volume;
      break;
    case Quantity::RHOUV:
      return rho * vel_x * _volume;
      break;
    case Quantity::RHOVV:
      return rho * vel_y * _volume;
      break;
    case Quantity::RHOWV:
      return rho * vel_z * _volume;
      break;
    case Quantity::RHOEV:
      return rho * E * _volume;
      break;
    case Quantity::P:
      return p;
      break;
    case Quantity::T:
      return T;
      break;
    case Quantity::VEL:
      return vel.norm();
      break;
    default:
      mooseError("Invalid 'quantity' parameter.");
  }
}

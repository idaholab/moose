//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StiffenedGasFluidProperties.h"

registerMooseObject("FluidPropertiesApp", StiffenedGasFluidProperties);

InputParameters
StiffenedGasFluidProperties::validParams()
{
  InputParameters params = StiffenedGasFluidPropertiesBase::validParams();

  params.addRequiredParam<Real>("gamma", "Heat capacity ratio");
  params.addRequiredParam<Real>("cv", "Constant-volume (isochoric) specific heat [J/(kg-K)]");
  params.addRequiredParam<Real>("q", "Reference specific internal energy [J/kg]");
  params.addRequiredParam<Real>("p_inf", "Stiffness pressure [Pa]");
  params.addParam<Real>("q_prime", 0, "Reference specific entropy [J/kg]");
  params.addParam<Real>("mu", 1.e-3, "Dynamic viscosity [Pa-s]");
  params.addParam<Real>("k", 0.6, "Thermal conductivity [W/(m-K)]");
  params.addParam<Real>("M", 0, "Molar mass [kg/mol]");
  params.addParam<Real>("T_c", 0, "Critical temperature [K]");
  params.addParam<Real>("rho_c", 0, "Critical density [kg/m^3]");
  params.addParam<Real>("e_c", 0, "Internal energy at the critical point [J/kg]");

  params.addClassDescription("Stiffened gas fluid properties from user-specified parameters.");

  return params;
}

StiffenedGasFluidProperties::StiffenedGasFluidProperties(const InputParameters & parameters)
  : StiffenedGasFluidPropertiesBase(parameters)
{
  _gamma = getParam<Real>("gamma");
  _cv = getParam<Real>("cv");
  if (_cv == 0.0)
    mooseError("cv cannot be zero.");
  _cp = _cv * _gamma;
  _q = getParam<Real>("q");
  _q_prime = getParam<Real>("q_prime");
  _p_inf = getParam<Real>("p_inf");
  _mu = getParam<Real>("mu");
  _k = getParam<Real>("k");
  _molar_mass = getParam<Real>("M");
  _T_c = getParam<Real>("T_c");
  _rho_c = getParam<Real>("rho_c");
  _e_c = getParam<Real>("e_c");
}

void
StiffenedGasFluidProperties::initialSetupInner()
{
  // do nothing since parameters have already been set in the constructor
}

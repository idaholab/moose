//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"

/**
 * Application used when running the subchannel module directly
 */
class SubChannelApp : public MooseApp
{
public:
  SubChannelApp(const InputParameters & parameters);

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

public:
  static InputParameters validParams();

  inline static const std::string MASS_FLOW_RATE = "mdot";
  inline static const std::string SURFACE_AREA = "S";
  inline static const std::string SUM_CROSSFLOW = "SumWij";
  inline static const std::string PRESSURE = "P";
  inline static const std::string PRESSURE_DROP = "DP";
  inline static const std::string ENTHALPY = "h";
  inline static const std::string TEMPERATURE = "T";
  inline static const std::string PIN_TEMPERATURE = "Tpin";
  inline static const std::string PIN_DIAMETER = "Dpin";
  inline static const std::string DENSITY = "rho";
  inline static const std::string VISCOSITY = "mu";
  inline static const std::string WETTED_PERIMETER = "w_perim";
  inline static const std::string LINEAR_HEAT_RATE = "q_prime";
  inline static const std::string DUCT_HEAT_FLUX = "duct_heat_flux";
  inline static const std::string DUCT_TEMPERATURE = "Tduct";
  inline static const std::string DISPLACEMENT = "displacement";
  inline static const std::string FRICTION_FACTOR = "ff";
};

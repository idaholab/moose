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

  /// mass flow rate
  static const std::string MASS_FLOW_RATE;
  /// surface area
  static const std::string SURFACE_AREA;
  /// sum of diversion crossflow
  static const std::string SUM_CROSSFLOW;
  /// pressure
  static const std::string PRESSURE;
  /// pressure drop
  static const std::string PRESSURE_DROP;
  /// enthalpy
  static const std::string ENTHALPY;
  /// temperature
  static const std::string TEMPERATURE;
  /// pin temperature
  static const std::string PIN_TEMPERATURE;
  /// pin diameter
  static const std::string PIN_DIAMETER;
  /// density
  static const std::string DENSITY;
  /// viscosity
  static const std::string VISCOSITY;
  /// wetted perimeter
  static const std::string WETTED_PERIMETER;
  /// linear heat rate
  static const std::string LINEAR_HEAT_RATE;
  /// duct linear heat rate
  static const std::string DUCT_HEAT_FLUX;
  /// duct temperature
  static const std::string DUCT_TEMPERATURE;
  /// subchannel displacement
  static const std::string DISPLACEMENT;
};

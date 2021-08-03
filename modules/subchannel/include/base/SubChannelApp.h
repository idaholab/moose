#pragma once

#include "MooseApp.h"

class SubChannelApp : public MooseApp
{
public:
  SubChannelApp(InputParameters parameters);
  virtual ~SubChannelApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

public:
  static InputParameters validParams();

  /// mass flow rate
  static const std::string MASS_FLOW_RATE;
  /// entropy
  static const std::string ENTROPY;
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
  /// density
  static const std::string DENSITY;
  /// viscosity
  static const std::string VISCOSITY;
  /// wetted perimeter
  static const std::string WETTED_PERIMETER;
  /// linear heat rate
  static const std::string LINEAR_HEAT_RATE;
};

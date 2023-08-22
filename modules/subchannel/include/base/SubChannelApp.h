/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#pragma once

#include "MooseApp.h"

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
  static const std::string DUCT_LINEAR_HEAT_RATE;
  /// duct temperature
  static const std::string DUCT_TEMPERATURE;
};

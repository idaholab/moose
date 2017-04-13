/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef WATER_STEAM_EOSAPP_H
#define WATER_STEAM_EOSAPP_H

#include "MooseApp.h"

class WaterSteamEOSApp;

template <>
InputParameters validParams<WaterSteamEOSApp>();

class WaterSteamEOSApp : public MooseApp
{
public:
  WaterSteamEOSApp(const InputParameters & parameters);
  virtual ~WaterSteamEOSApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* WATER_STEAM_EOSAPP_H */

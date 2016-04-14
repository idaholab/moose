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

template<>
InputParameters validParams<WaterSteamEOSApp>();

class WaterSteamEOSApp : public MooseApp
{
public:
  WaterSteamEOSApp(const InputParameters & parameters);
  virtual ~WaterSteamEOSApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

private:
  /// Used to prevent re-registration of objects
  static bool _registered_objects;
  /// Used to prevent re-association of syntax
  static bool _associated_syntax;
};

#endif /* WATER_STEAM_EOSAPP_H */

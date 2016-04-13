/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HEAT_CONDUCTIONAPP_H
#define HEAT_CONDUCTIONAPP_H

#include "MooseApp.h"

class HeatConductionApp;

template<>
InputParameters validParams<HeatConductionApp>();

class HeatConductionApp : public MooseApp
{
public:
  HeatConductionApp(const InputParameters & parameters);
  virtual ~HeatConductionApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

private:
  /// Used to prevent re-registration of objects
  static bool _registered_objects;
  /// Used to prevent re-association of syntax
  static bool _associated_syntax;
};

#endif /* HEAT_CONDUCTIONAPP_H */

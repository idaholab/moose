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

template <>
InputParameters validParams<HeatConductionApp>();

class HeatConductionApp : public MooseApp
{
public:
  HeatConductionApp(const InputParameters & parameters);
  virtual ~HeatConductionApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* HEAT_CONDUCTIONAPP_H */

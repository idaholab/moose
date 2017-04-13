/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef FLUIDPROPERTIESAPP_H
#define FLUIDPROPERTIESAPP_H

#include "MooseApp.h"

class FluidPropertiesApp;

template <>
InputParameters validParams<FluidPropertiesApp>();

class FluidPropertiesApp : public MooseApp
{
public:
  FluidPropertiesApp(InputParameters parameters);
  virtual ~FluidPropertiesApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* FLUIDPROPERTIESAPP_H */

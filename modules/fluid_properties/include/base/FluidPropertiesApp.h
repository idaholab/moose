//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags(Factory & factory);
};

#endif /* FLUIDPROPERTIESAPP_H */

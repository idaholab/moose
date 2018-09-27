//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef FLUIDPROPERTIESTESTAPP_H
#define FLUIDPROPERTIESTESTAPP_H

#include "MooseApp.h"

class FluidPropertiesTestApp;

template <>
InputParameters validParams<FluidPropertiesTestApp>();

class FluidPropertiesTestApp : public MooseApp
{
public:
  FluidPropertiesTestApp(InputParameters parameters);
  virtual ~FluidPropertiesTestApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags(Factory & factory);
};

#endif /* FLUIDPROPERTIESTESTAPP_H */

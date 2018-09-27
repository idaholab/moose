//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef RICHARDSTESTAPP_H
#define RICHARDSTESTAPP_H

#include "MooseApp.h"

class RichardsTestApp;

template <>
InputParameters validParams<RichardsTestApp>();

class RichardsTestApp : public MooseApp
{
public:
  RichardsTestApp(InputParameters parameters);
  virtual ~RichardsTestApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags(Factory & factory);
};

#endif /* RICHARDSTESTAPP_H */

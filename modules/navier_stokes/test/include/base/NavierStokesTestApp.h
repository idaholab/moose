//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef NAVIERSTOKESTESTAPP_H
#define NAVIERSTOKESTESTAPP_H

#include "MooseApp.h"

class NavierStokesTestApp;

template <>
InputParameters validParams<NavierStokesTestApp>();

class NavierStokesTestApp : public MooseApp
{
public:
  NavierStokesTestApp(InputParameters parameters);
  virtual ~NavierStokesTestApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags(Factory & factory);
};

#endif /* NAVIERSTOKESTESTAPP_H */

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef STORKTESTAPP_H
#define STORKTESTAPP_H

#include "MooseApp.h"

class StorkTestApp;

template <>
InputParameters validParams<StorkTestApp>();

class StorkTestApp : public MooseApp
{
public:
  StorkTestApp(InputParameters parameters);
  virtual ~StorkTestApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
};

#endif /* STORKTESTAPP_H */

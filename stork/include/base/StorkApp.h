//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef STORKAPP_H
#define STORKAPP_H

#include "MooseApp.h"

class StorkApp;

template <>
InputParameters validParams<StorkApp>();

class StorkApp : public MooseApp
{
public:
  StorkApp(InputParameters parameters);
  virtual ~StorkApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};

#endif /* STORKAPP_H */

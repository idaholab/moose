//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FUNCTIONAL_EXPANSION_TOOLSAPP_H
#define FUNCTIONAL_EXPANSION_TOOLSAPP_H

#include "MooseApp.h"

class FunctionalExpansionToolsApp;

template <>
InputParameters validParams<FunctionalExpansionToolsApp>();

class FunctionalExpansionToolsApp : public MooseApp
{
public:
  FunctionalExpansionToolsApp(InputParameters parameters);
  virtual ~FunctionalExpansionToolsApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* FUNCTIONAL_EXPANSION_TOOLSAPP_H */

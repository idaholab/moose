//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChemicalReactionsApp.h"

class ChemicalReactionsTestApp : public ChemicalReactionsApp
{
public:
  static InputParameters validParams();

  ChemicalReactionsTestApp(const InputParameters & parameters);
  virtual ~ChemicalReactionsTestApp();

  static void
  registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objects = false);
  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags(Factory & factory);
};

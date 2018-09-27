//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TENSOR_MECHANICSAPP_H
#define TENSOR_MECHANICSAPP_H

#include "MooseApp.h"

class TensorMechanicsApp;

template <>
InputParameters validParams<TensorMechanicsApp>();

class TensorMechanicsApp : public MooseApp
{
public:
  TensorMechanicsApp(const InputParameters & parameters);
  virtual ~TensorMechanicsApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags(Factory & factory);
};

#endif /* TENSOR_MECHANICSAPP_H */

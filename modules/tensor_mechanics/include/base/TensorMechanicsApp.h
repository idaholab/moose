/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* TENSOR_MECHANICSAPP_H */

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

template<>
InputParameters validParams<TensorMechanicsApp>();

class TensorMechanicsApp : public MooseApp
{
public:
  TensorMechanicsApp(const InputParameters & parameters);
  virtual ~TensorMechanicsApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

private:
  /// Used to prevent re-registration of objects
  static bool _registered_objects;
  /// Used to prevent re-association of syntax
  static bool _associated_syntax;
};

#endif /* TENSOR_MECHANICSAPP_H */

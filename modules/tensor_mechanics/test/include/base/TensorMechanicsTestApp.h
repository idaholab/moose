#ifndef TENSORMECHANICSTESTAPP_H
#define TENSORMECHANICSTESTAPP_H

#include "MooseApp.h"

class TensorMechanicsTestApp;

template <>
InputParameters validParams<TensorMechanicsTestApp>();

class TensorMechanicsTestApp : public MooseApp
{
public:
  TensorMechanicsTestApp(InputParameters parameters);
  virtual ~TensorMechanicsTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* TENSORMECHANICSTESTAPP_H */

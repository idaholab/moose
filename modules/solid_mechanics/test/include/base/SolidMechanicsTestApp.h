#ifndef SOLIDMECHANICSTESTAPP_H
#define SOLIDMECHANICSTESTAPP_H

#include "MooseApp.h"

class SolidMechanicsTestApp;

template <>
InputParameters validParams<SolidMechanicsTestApp>();

class SolidMechanicsTestApp : public MooseApp
{
public:
  SolidMechanicsTestApp(InputParameters parameters);
  virtual ~SolidMechanicsTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* SOLIDMECHANICSTESTAPP_H */

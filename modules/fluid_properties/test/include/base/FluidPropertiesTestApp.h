#ifndef FLUIDPROPERTIESTESTAPP_H
#define FLUIDPROPERTIESTESTAPP_H

#include "FluidPropertiesApp.h"

class FluidPropertiesTestApp;

template <>
InputParameters validParams<FluidPropertiesTestApp>();

class FluidPropertiesTestApp : public FluidPropertiesApp
{
public:
  FluidPropertiesTestApp(InputParameters parameters);
  virtual ~FluidPropertiesTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* FLUIDPROPERTIESTESTAPP_H */

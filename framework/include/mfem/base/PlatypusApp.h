#pragma once

#include "MooseApp.h"

class PlatypusApp : public MooseApp
{
public:
  static InputParameters validParams();

  PlatypusApp(InputParameters parameters);
  virtual ~PlatypusApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

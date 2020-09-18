#pragma once

#include "MooseApp.h"

class isopodApp : public MooseApp
{
public:
  static InputParameters validParams();

  isopodApp(InputParameters parameters);
  virtual ~isopodApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};

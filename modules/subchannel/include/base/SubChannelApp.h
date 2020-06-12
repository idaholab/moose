#pragma once

#include "MooseApp.h"

class SubChannelApp : public MooseApp
{
public:
  SubChannelApp(InputParameters parameters);
  virtual ~SubChannelApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

public:
  static InputParameters validParams();
};

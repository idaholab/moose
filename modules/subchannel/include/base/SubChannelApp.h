#pragma once

#include "MooseApp.h"

class SubChannelApp;

template <>
InputParameters validParams<SubChannelApp>();

class SubChannelApp : public MooseApp
{
public:
  SubChannelApp(InputParameters parameters);
  virtual ~SubChannelApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};

#pragma once

#include "MooseApp.h"

class SubChannelTestApp;

template <>
InputParameters validParams<SubChannelTestApp>();

class SubChannelTestApp : public MooseApp
{
public:
  SubChannelTestApp(InputParameters parameters);
  virtual ~SubChannelTestApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
};

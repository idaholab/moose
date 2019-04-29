#pragma once

#include "THMApp.h"

class THMTestApp;

template <>
InputParameters validParams<THMTestApp>();

class THMTestApp : public THMApp
{
public:
  THMTestApp(InputParameters parameters);
  virtual ~THMTestApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
};

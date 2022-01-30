#pragma once

#include "ThermalHydraulicsApp.h"

class ThermalHydraulicsTestApp : public ThermalHydraulicsApp
{
public:
  ThermalHydraulicsTestApp(InputParameters parameters);
  virtual ~ThermalHydraulicsTestApp();

public:
  static InputParameters validParams();
  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
};

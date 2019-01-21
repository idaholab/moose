#ifndef THMTESTAPP_H
#define THMTESTAPP_H

#include "MooseApp.h"

class THMTestApp;

template <>
InputParameters validParams<THMTestApp>();

class THMTestApp : public MooseApp
{
public:
  THMTestApp(InputParameters parameters);
  virtual ~THMTestApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
};

#endif /* THMTESTAPP_H */

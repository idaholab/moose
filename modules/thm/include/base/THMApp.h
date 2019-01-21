#ifndef THMAPP_H
#define THMAPP_H

#include "MooseApp.h"

class THMApp;

template <>
InputParameters validParams<THMApp>();

class THMApp : public MooseApp
{
public:
  THMApp(InputParameters parameters);

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};

#endif /* THMAPP_H */

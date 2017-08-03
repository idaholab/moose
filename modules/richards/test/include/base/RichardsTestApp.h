#ifndef RICHARDSTESTAPP_H
#define RICHARDSTESTAPP_H

#include "RichardsApp.h"

class RichardsTestApp;

template <>
InputParameters validParams<RichardsTestApp>();

class RichardsTestApp : public RichardsApp
{
public:
  RichardsTestApp(InputParameters parameters);
  virtual ~RichardsTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* RICHARDSTESTAPP_H */

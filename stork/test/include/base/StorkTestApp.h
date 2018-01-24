#ifndef STORKTESTAPP_H
#define STORKTESTAPP_H

#include "MooseApp.h"

class StorkTestApp;

template <>
InputParameters validParams<StorkTestApp>();

class StorkTestApp : public MooseApp
{
public:
  StorkTestApp(InputParameters parameters);
  virtual ~StorkTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* STORKTESTAPP_H */

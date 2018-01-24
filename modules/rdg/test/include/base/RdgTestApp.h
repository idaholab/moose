#ifndef RDGTESTAPP_H
#define RDGTESTAPP_H

#include "MooseApp.h"

class RdgTestApp;

template <>
InputParameters validParams<RdgTestApp>();

class RdgTestApp : public MooseApp
{
public:
  RdgTestApp(InputParameters parameters);
  virtual ~RdgTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* RDGTESTAPP_H */

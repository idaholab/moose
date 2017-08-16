#ifndef POROUSFLOWTESTAPP_H
#define POROUSFLOWTESTAPP_H

#include "MooseApp.h"

class PorousFlowTestApp;

template <>
InputParameters validParams<PorousFlowTestApp>();

class PorousFlowTestApp : public MooseApp
{
public:
  PorousFlowTestApp(InputParameters parameters);
  virtual ~PorousFlowTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* POROUSFLOWTESTAPP_H */

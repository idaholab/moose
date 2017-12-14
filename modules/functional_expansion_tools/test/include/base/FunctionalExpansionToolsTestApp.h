#ifndef FUNCTIONAL_EXPANSION_TOOLSTESTAPP_H
#define FUNCTIONAL_EXPANSION_TOOLSTESTAPP_H

#include "MooseApp.h"

class FunctionalExpansionToolsTestApp;

template <>
InputParameters validParams<FunctionalExpansionToolsTestApp>();

class FunctionalExpansionToolsTestApp : public MooseApp
{
public:
  FunctionalExpansionToolsTestApp(InputParameters parameters);
  virtual ~FunctionalExpansionToolsTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* FUNCTIONAL_EXPANSION_TOOLSTESTAPP_H */

#ifndef FUNCTIONAL_EXPANSION_TOOLSAPP_H
#define FUNCTIONAL_EXPANSION_TOOLSAPP_H

#include "MooseApp.h"

class FunctionalExpansionToolsApp;

template <>
InputParameters validParams<FunctionalExpansionToolsApp>();

class FunctionalExpansionToolsApp : public MooseApp
{
public:
  FunctionalExpansionToolsApp(InputParameters parameters);
  virtual ~FunctionalExpansionToolsApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void registerObjectDepends(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* FUNCTIONAL_EXPANSION_TOOLSAPP_H */

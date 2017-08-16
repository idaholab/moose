#ifndef CHEMICALREACTIONSTESTAPP_H
#define CHEMICALREACTIONSTESTAPP_H

#include "MooseApp.h"

class ChemicalReactionsTestApp;

template <>
InputParameters validParams<ChemicalReactionsTestApp>();

class ChemicalReactionsTestApp : public MooseApp
{
public:
  ChemicalReactionsTestApp(InputParameters parameters);
  virtual ~ChemicalReactionsTestApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
};

#endif /* CHEMICALREACTIONSTESTAPP_H */

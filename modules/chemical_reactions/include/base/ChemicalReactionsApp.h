/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHEMICAL_REACTIONSAPP_H
#define CHEMICAL_REACTIONSAPP_H

#include "MooseApp.h"

class ChemicalReactionsApp;

template<>
InputParameters validParams<ChemicalReactionsApp>();

class ChemicalReactionsApp : public MooseApp
{
public:
  ChemicalReactionsApp(const InputParameters & parameters);
  virtual ~ChemicalReactionsApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

private:
  /// Used to prevent re-registration of objects
  static bool _registered_objects;
  /// Used to prevent re-association of syntax
  static bool _associated_syntax;
};

#endif /* CHEMICAL_REACTIONSAPP_H */

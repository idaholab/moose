/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MISCAPP_H
#define MISCAPP_H

#include "MooseApp.h"

class MiscApp;

template<>
InputParameters validParams<MiscApp>();

class MiscApp : public MooseApp
{
public:
  MiscApp(const InputParameters & parameters);
  virtual ~MiscApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

private:
  /// Used to prevent re-registration of objects
  static bool _registered_objects;
  /// Used to prevent re-association of syntax
  static bool _associated_syntax;
};

#endif /* MISCAPP_H */

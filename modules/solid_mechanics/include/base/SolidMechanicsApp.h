/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef SOLID_MECHANICSAPP_H
#define SOLID_MECHANICSAPP_H

#include "MooseApp.h"

class SolidMechanicsApp;

template<>
InputParameters validParams<SolidMechanicsApp>();

class SolidMechanicsApp : public MooseApp
{
public:
  SolidMechanicsApp(const InputParameters & parameters);

  virtual ~SolidMechanicsApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

private:
  /// Used to prevent re-registration of objects
  static bool _registered_objects;
  /// Used to prevent re-association of syntax
  static bool _associated_syntax;
};

#endif /* SOLID_MECHANICSAPP_H */

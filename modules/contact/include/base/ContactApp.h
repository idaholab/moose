/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CONTACTAPP_H
#define CONTACTAPP_H

#include "MooseApp.h"

class ContactApp;

template<>
InputParameters validParams<ContactApp>();

class ContactApp : public MooseApp
{
public:
  ContactApp(const InputParameters & parameters);
  virtual ~ContactApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

private:
  /// Used to prevent re-registration of objects
  static bool _registered_objects;
  /// Used to prevent re-association of syntax
  static bool _associated_syntax;
};

#endif /* CONTACTAPP_H */

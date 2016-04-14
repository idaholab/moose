/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef NAVIER_STOKESAPP_H
#define NAVIER_STOKESAPP_H

#include "MooseApp.h"

class NavierStokesApp;

template<>
InputParameters validParams<NavierStokesApp>();

class NavierStokesApp : public MooseApp
{
public:
  NavierStokesApp(InputParameters parameters);
  virtual ~NavierStokesApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

private:
  /// Used to prevent re-registration of objects
  static bool _registered_objects;
  /// Used to prevent re-association of syntax
  static bool _associated_syntax;
};

#endif /* NAVIER_STOKESAPP_H */

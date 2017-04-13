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

template <>
InputParameters validParams<NavierStokesApp>();

class NavierStokesApp : public MooseApp
{
public:
  NavierStokesApp(InputParameters parameters);
  virtual ~NavierStokesApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* NAVIER_STOKESAPP_H */

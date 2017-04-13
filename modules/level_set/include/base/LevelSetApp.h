/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef LEVELSETAPP_H
#define LEVELSETAPP_H

#include "MooseApp.h"

class LevelSetApp;

template <>
InputParameters validParams<LevelSetApp>();

class LevelSetApp : public MooseApp
{
public:
  LevelSetApp(InputParameters parameters);

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* LEVELSETAPP_H */

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RDGAPP_H
#define RDGAPP_H

#include "MooseApp.h"

class RdgApp;

template <>
InputParameters validParams<RdgApp>();

class RdgApp : public MooseApp
{
public:
  RdgApp(InputParameters parameters);
  virtual ~RdgApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* RDGAPP_H */

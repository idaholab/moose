/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMBINEDAPP_H
#define COMBINEDAPP_H

#include "MooseApp.h"

class CombinedApp;

template <>
InputParameters validParams<CombinedApp>();

class CombinedApp : public MooseApp
{
public:
  CombinedApp(const InputParameters & parameters);
  virtual ~CombinedApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* COMBINEDAPP_H_ */

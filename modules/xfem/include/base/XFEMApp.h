/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMAPP_H
#define XFEMAPP_H

#include "MooseApp.h"

class XFEMApp;

template <>
InputParameters validParams<XFEMApp>();

class XFEMApp : public MooseApp
{
public:
  XFEMApp(const InputParameters & parameters);
  virtual ~XFEMApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* XFEMAPP_H */

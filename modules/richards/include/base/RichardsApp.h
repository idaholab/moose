/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef RICHARDSAPP_H
#define RICHARDSAPP_H

#include "MooseApp.h"

class RichardsApp;

template<>
InputParameters validParams<RichardsApp>();

/**
 * The Richards equation is a nonlinear diffusion
 * equation that models multiphase flow through
 * porous materials
 */
class RichardsApp : public MooseApp
{
public:
  RichardsApp(const InputParameters & parameters);
  virtual ~RichardsApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);

private:
  /// Used to prevent re-registration of objects
  static bool _registered_objects;
  /// Used to prevent re-association of syntax
  static bool _associated_syntax;
};

#endif /* RICHARDSAPP_H */

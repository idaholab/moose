/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PHASE_FIELDAPP_H
#define PHASE_FIELDAPP_H

#include "MooseApp.h"

class PhaseFieldApp;

template <>
InputParameters validParams<PhaseFieldApp>();

class PhaseFieldApp : public MooseApp
{
public:
  PhaseFieldApp(const InputParameters & parameters);
  virtual ~PhaseFieldApp();

  static void registerApps();
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags();
};

#endif /* PHASE_FIELDAPP_H */

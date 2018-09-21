//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags(Factory & factory);
};

#endif /* PHASE_FIELDAPP_H */

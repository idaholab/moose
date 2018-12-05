//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifndef EXTERNAL_PETSC_SOLVERAPP_H
#define EXTERNAL_PETSC_SOLVERAPP_H

#include "MooseApp.h"

class ExternalPetscSolverApp;

template <>
InputParameters validParams<ExternalPetscSolverApp>();

class ExternalPetscSolverApp : public MooseApp
{
public:
  ExternalPetscSolverApp(InputParameters parameters);
  virtual ~ExternalPetscSolverApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

#if LIBMESH_HAVE_PETSC
  TS & getExternalPETScTS() { return _ts; }
#endif
private:
#if LIBMESH_HAVE_PETSC
  TS _ts;
#endif
};

#endif /* EXTERNAL_PETSC_SOLVERAPP_H */

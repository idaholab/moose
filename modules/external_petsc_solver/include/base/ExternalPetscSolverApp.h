//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"

/**
 * This is a demo used to demonstrate how to couple an external app
 * through the MOOSE wrapper APP.
 * We are using a PETSc application as an example. ExternalPetscSolverApp
 * create and destroys an external PETSc FEM/FDM solver.
 */
class ExternalPetscSolverApp : public MooseApp
{
public:
  static InputParameters validParams();

  ExternalPetscSolverApp(const InputParameters & parameters);
  virtual ~ExternalPetscSolverApp();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);

  TS & getPetscTS();

  bool isPetscApp() const { return _is_petsc_app; }

private:
  TS _ts;
  bool _is_petsc_app;
};

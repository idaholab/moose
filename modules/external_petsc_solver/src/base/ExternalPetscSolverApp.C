//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExternalPetscSolverApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "PETScDiffusionFDM.h"

template <>
InputParameters
validParams<ExternalPetscSolverApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ExternalPetscSolverApp::ExternalPetscSolverApp(InputParameters parameters) : MooseApp(parameters)
{
  ExternalPetscSolverApp::registerAll(_factory, _action_factory, _syntax);

#if LIBMESH_HAVE_PETSC
  PETScExternalSolverCreate(_comm->get(), &_ts);
#else
  mooseError("You need to have PETSc installed to use ExternalPETScApp");
#endif
}

ExternalPetscSolverApp::~ExternalPetscSolverApp()
{
#if LIBMESH_HAVE_PETSC
  PETScExternalSolverDestroy(_ts);
#endif
}

void
ExternalPetscSolverApp::registerAll(Factory & f, ActionFactory & af, Syntax & /*s*/)
{
  Registry::registerObjectsTo(f, {"ExternalPetscSolverApp"});
  Registry::registerActionsTo(af, {"ExternalPetscSolverApp"});

  /* register custom execute flags, action syntax, etc. here */
}

void
ExternalPetscSolverApp::registerApps()
{
  registerApp(ExternalPetscSolverApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
extern "C" void
ExternalPetscSolverApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ExternalPetscSolverApp::registerAll(f, af, s);
}
extern "C" void
ExternalPetscSolverApp__registerApps()
{
  ExternalPetscSolverApp::registerApps();
}

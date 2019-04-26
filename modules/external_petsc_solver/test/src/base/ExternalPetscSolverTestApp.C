//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExternalPetscSolverTestApp.h"
#include "ExternalPetscSolverApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<ExternalPetscSolverTestApp>()
{
  InputParameters params = validParams<ExternalPetscSolverApp>();
  return params;
}

ExternalPetscSolverTestApp::ExternalPetscSolverTestApp(InputParameters parameters)
  : MooseApp(parameters)
{
  ExternalPetscSolverTestApp::registerAll(
      _factory, _action_factory, _syntax, getParam<bool>("allow_test_objects"));
}

ExternalPetscSolverTestApp::~ExternalPetscSolverTestApp() {}

void
ExternalPetscSolverTestApp::registerAll(Factory & f,
                                        ActionFactory & af,
                                        Syntax & s,
                                        bool use_test_objs)
{
  ExternalPetscSolverApp::registerAll(f, af, s);
  if (use_test_objs)
  {
    Registry::registerObjectsTo(f, {"ExternalPetscSolverTestApp"});
    Registry::registerActionsTo(af, {"ExternalPetscSolverTestApp"});
  }
}

void
ExternalPetscSolverTestApp::registerApps()
{
  registerApp(ExternalPetscSolverApp);
  registerApp(ExternalPetscSolverTestApp);
}

/***************************************************************************************************
 *********************** Dynamic Library Entry Points - DO NOT MODIFY ******************************
 **************************************************************************************************/
// External entry point for dynamic application loading
extern "C" void
ExternalPetscSolverTestApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ExternalPetscSolverTestApp::registerAll(f, af, s);
}
extern "C" void
ExternalPetscSolverTestApp__registerApps()
{
  ExternalPetscSolverTestApp::registerApps();
}

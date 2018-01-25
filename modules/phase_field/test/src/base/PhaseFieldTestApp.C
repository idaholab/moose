//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "PhaseFieldTestApp.h"
#include "PhaseFieldApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "GaussContForcing.h"

template <>
InputParameters
validParams<PhaseFieldTestApp>()
{
  InputParameters params = validParams<PhaseFieldApp>();
  return params;
}

PhaseFieldTestApp::PhaseFieldTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  PhaseFieldApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  PhaseFieldApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  PhaseFieldApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    PhaseFieldTestApp::registerObjects(_factory);
    PhaseFieldTestApp::associateSyntax(_syntax, _action_factory);
  }
}

PhaseFieldTestApp::~PhaseFieldTestApp() {}

// External entry point for dynamic application loading
extern "C" void
PhaseFieldTestApp__registerApps()
{
  PhaseFieldTestApp::registerApps();
}
void
PhaseFieldTestApp::registerApps()
{
  registerApp(PhaseFieldApp);
  registerApp(PhaseFieldTestApp);
}

// External entry point for dynamic object registration
extern "C" void
PhaseFieldTestApp__registerObjects(Factory & factory)
{
  PhaseFieldTestApp::registerObjects(factory);
}
void
PhaseFieldTestApp::registerObjects(Factory & factory)
{
  registerKernel(GaussContForcing);
}

// External entry point for dynamic syntax association
extern "C" void
PhaseFieldTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  PhaseFieldTestApp::associateSyntax(syntax, action_factory);
}
void
PhaseFieldTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
PhaseFieldTestApp__registerExecFlags(Factory & factory)
{
  PhaseFieldTestApp::registerExecFlags(factory);
}
void
PhaseFieldTestApp::registerExecFlags(Factory & /*factory*/)
{
}

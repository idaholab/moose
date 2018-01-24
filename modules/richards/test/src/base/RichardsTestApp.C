//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "RichardsTestApp.h"
#include "RichardsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<RichardsTestApp>()
{
  InputParameters params = validParams<RichardsApp>();
  return params;
}

RichardsTestApp::RichardsTestApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  RichardsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  RichardsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  RichardsApp::registerExecFlags(_factory);

  bool use_test_objs = getParam<bool>("allow_test_objects");
  if (use_test_objs)
  {
    RichardsTestApp::registerObjects(_factory);
    RichardsTestApp::associateSyntax(_syntax, _action_factory);
  }

  mooseDeprecated("Please use the PorousFlow module instead.  If Richards contains functionality "
                  "not included in PorousFlow, please contact the moose-users google group");
}

RichardsTestApp::~RichardsTestApp() {}

// External entry point for dynamic application loading
extern "C" void
RichardsTestApp__registerApps()
{
  RichardsTestApp::registerApps();
}
void
RichardsTestApp::registerApps()
{
  registerApp(RichardsApp);
  registerApp(RichardsTestApp);
}

// External entry point for dynamic object registration
extern "C" void
RichardsTestApp__registerObjects(Factory & factory)
{
  RichardsTestApp::registerObjects(factory);
}
void
RichardsTestApp::registerObjects(Factory & /*factory*/)
{
}

// External entry point for dynamic syntax association
extern "C" void
RichardsTestApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  RichardsTestApp::associateSyntax(syntax, action_factory);
}
void
RichardsTestApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}

// External entry point for dynamic execute flag registration
extern "C" void
RichardsTestApp__registerExecFlags(Factory & factory)
{
  RichardsTestApp::registerExecFlags(factory);
}
void
RichardsTestApp::registerExecFlags(Factory & /*factory*/)
{
}

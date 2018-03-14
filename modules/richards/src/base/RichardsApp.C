//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RichardsApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<RichardsApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("RichardsApp");

RichardsApp::RichardsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  RichardsApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  RichardsApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  RichardsApp::registerExecFlags(_factory);

  mooseDeprecated("Please use the PorousFlow module instead.  If Richards contains functionality "
                  "not included in PorousFlow, please contact the moose-users google group");
}

RichardsApp::~RichardsApp() {}

void
RichardsApp::registerApps()
{
  registerApp(RichardsApp);
}

void
RichardsApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"RichardsApp"});
}

void
RichardsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"RichardsApp"});

  registerSyntaxTask("Q2PAction", "Q2P", "add_kernel");
  registerSyntaxTask("Q2PAction", "Q2P", "add_aux_variable");
  registerSyntaxTask("Q2PAction", "Q2P", "add_function");
  registerSyntaxTask("Q2PAction", "Q2P", "add_postprocessor");
}

// External entry point for dynamic execute flag registration
extern "C" void
RichardsApp__registerExecFlags(Factory & factory)
{
  RichardsApp::registerExecFlags(factory);
}
void
RichardsApp::registerExecFlags(Factory & /*factory*/)
{
}

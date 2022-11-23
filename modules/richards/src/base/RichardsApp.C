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

InputParameters
RichardsApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;
  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("RichardsApp");

RichardsApp::RichardsApp(const InputParameters & parameters) : MooseApp(parameters)
{
  mooseDeprecated("Please use the PorousFlow module instead.  If Richards contains functionality "
                  "not included in PorousFlow, please contact the moose-users google group");
  RichardsApp::registerAll(_factory, _action_factory, _syntax);
}

RichardsApp::~RichardsApp() {}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntaxTask("Q2PAction", "Q2P", "add_kernel");
  registerSyntaxTask("Q2PAction", "Q2P", "add_aux_variable");
  registerSyntaxTask("Q2PAction", "Q2P", "add_function");
  registerSyntaxTask("Q2PAction", "Q2P", "add_postprocessor");
}
void
RichardsApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"RichardsApp"});
  Registry::registerActionsTo(af, {"RichardsApp"});
  associateSyntaxInner(s, af);
}

void
RichardsApp::registerApps()
{
  registerApp(RichardsApp);
}

void
RichardsApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"RichardsApp"});
}

void
RichardsApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"RichardsApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
RichardsApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

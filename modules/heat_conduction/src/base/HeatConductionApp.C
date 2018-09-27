//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

template <>
InputParameters
validParams<HeatConductionApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("HeatConductionApp");

HeatConductionApp::HeatConductionApp(const InputParameters & parameters) : MooseApp(parameters)
{
  HeatConductionApp::registerAll(_factory, _action_factory, _syntax);
}

HeatConductionApp::~HeatConductionApp() {}

void
HeatConductionApp::registerApps()
{
  registerApp(HeatConductionApp);
}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  // This registers an action to add the "slave_flux" vector to the system at the right time
  registerTask("add_slave_flux_vector", false);
  addTaskDependency("add_slave_flux_vector", "ready_to_init");
  addTaskDependency("init_problem", "add_slave_flux_vector");
  registerSyntax("AddSlaveFluxVectorAction", "ThermalContact/*");

  registerSyntaxTask("ThermalContactAuxBCsAction", "ThermalContact/*", "add_aux_kernel");
  registerSyntaxTask("ThermalContactAuxVarsAction", "ThermalContact/*", "add_aux_variable");
  registerSyntaxTask("ThermalContactBCsAction", "ThermalContact/*", "add_bc");
  registerSyntaxTask("ThermalContactDiracKernelsAction", "ThermalContact/*", "add_dirac_kernel");
  registerSyntaxTask("ThermalContactMaterialsAction", "ThermalContact/*", "add_material");
}

void
HeatConductionApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"HeatConductionApp"});
  Registry::registerActionsTo(af, {"HeatConductionApp"});
  associateSyntaxInner(s, af);
}

void
HeatConductionApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"HeatConductionApp"});
}

void
HeatConductionApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"HeatConductionApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
HeatConductionApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("use registerAll instead of registerExecFlags");
}

extern "C" void
HeatConductionApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  HeatConductionApp::registerAll(f, af, s);
}
extern "C" void
HeatConductionApp__registerApps()
{
  HeatConductionApp::registerApps();
}

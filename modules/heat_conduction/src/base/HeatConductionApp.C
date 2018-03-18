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
  Moose::registerObjects(_factory);
  HeatConductionApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  HeatConductionApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  HeatConductionApp::registerExecFlags(_factory);
}

HeatConductionApp::~HeatConductionApp() {}

// External entry point for dynamic application loading
extern "C" void
HeatConductionApp__registerApps()
{
  HeatConductionApp::registerApps();
}
void
HeatConductionApp::registerApps()
{
  registerApp(HeatConductionApp);
}

// External entry point for dynamic object registration
extern "C" void
HeatConductionApp__registerObjects(Factory & factory)
{
  HeatConductionApp::registerObjects(factory);
}
void
HeatConductionApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"HeatConductionApp"});
}

// External entry point for dynamic syntax association
extern "C" void
HeatConductionApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  HeatConductionApp::associateSyntax(syntax, action_factory);
}
void
HeatConductionApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"HeatConductionApp"});

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

// External entry point for dynamic execute flag registration
extern "C" void
HeatConductionApp__registerExecFlags(Factory & factory)
{
  HeatConductionApp::registerExecFlags(factory);
}
void
HeatConductionApp::registerExecFlags(Factory & /*factory*/)
{
}

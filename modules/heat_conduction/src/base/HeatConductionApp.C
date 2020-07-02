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

InputParameters
HeatConductionApp::validParams()
{
  InputParameters params = ::validParams<MooseApp>();

  // Do not use legacy DirichletBC, that is, set DirichletBC default for preset = true
  params.set<bool>("use_legacy_dirichlet_bc") = false;

  params.set<bool>("use_legacy_material_output") = false;

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
  // This registers an action to add the "secondary_flux" vector to the system at the right time
  registerTask("add_secondary_flux_vector", false);
  addTaskDependency("add_secondary_flux_vector", "ready_to_init");
  addTaskDependency("init_problem", "add_secondary_flux_vector");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_aux_kernel");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_aux_variable");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_bc");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_dirac_kernel");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_material");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_secondary_flux_vector");

  registerSyntaxTask("RadiationTransferAction", "GrayDiffuseRadiation/*", "add_mesh_generator");
  registerSyntaxTask("RadiationTransferAction", "GrayDiffuseRadiation/*", "setup_mesh_complete");
  registerSyntaxTask("RadiationTransferAction", "GrayDiffuseRadiation/*", "add_user_object");
  registerSyntaxTask("RadiationTransferAction", "GrayDiffuseRadiation/*", "add_bc");
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

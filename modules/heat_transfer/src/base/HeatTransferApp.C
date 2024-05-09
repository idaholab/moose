//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatTransferApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"
#include "RayTracingApp.h"

InputParameters
HeatTransferApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("HeatTransferApp");

HeatTransferApp::HeatTransferApp(const InputParameters & parameters) : MooseApp(parameters)
{
  HeatTransferApp::registerAll(_factory, _action_factory, _syntax);
}

HeatTransferApp::~HeatTransferApp() {}

void
HeatTransferApp::registerApps()
{
  registerApp(HeatTransferApp);

  RayTracingApp::registerApps();
}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  // This registers an action to add the "secondary_flux" vector to the system at the right time
  registerTask("add_secondary_flux_vector", false);
  addTaskDependency("add_secondary_flux_vector", "ready_to_init");
  addTaskDependency("setup_dampers", "add_secondary_flux_vector");

  registerSyntax("HeatConductionCG", "Physics/HeatConduction/FiniteElement/*");
  registerSyntax("HeatConductionFV", "Physics/HeatConduction/FiniteVolume/*");

  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_aux_kernel");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_aux_variable");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_bc");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_dirac_kernel");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_material");
  registerSyntaxTask("ThermalContactAction", "ThermalContact/*", "add_secondary_flux_vector");

  registerSyntaxTask(
      "ThermalContactAction", "Modules/HeatTransfer/ThermalContact/BC/*", "add_aux_kernel");
  registerSyntaxTask(
      "ThermalContactAction", "Modules/HeatTransfer/ThermalContact/BC/*", "add_aux_variable");
  registerSyntaxTask("ThermalContactAction", "Modules/HeatTransfer/ThermalContact/BC/*", "add_bc");
  registerSyntaxTask(
      "ThermalContactAction", "Modules/HeatTransfer/ThermalContact/BC/*", "add_dirac_kernel");
  registerSyntaxTask(
      "ThermalContactAction", "Modules/HeatTransfer/ThermalContact/BC/*", "add_material");
  registerSyntaxTask("ThermalContactAction",
                     "Modules/HeatTransfer/ThermalContact/BC/*",
                     "add_secondary_flux_vector");

  registerSyntaxTask("RadiationTransferAction", "GrayDiffuseRadiation/*", "append_mesh_generator");
  registerSyntaxTask("RadiationTransferAction", "GrayDiffuseRadiation/*", "setup_mesh_complete");
  registerSyntaxTask("RadiationTransferAction", "GrayDiffuseRadiation/*", "add_user_object");
  registerSyntaxTask("RadiationTransferAction", "GrayDiffuseRadiation/*", "add_bc");
  registerSyntaxTask(
      "RadiationTransferAction", "GrayDiffuseRadiation/*", "add_ray_boundary_condition");

  registerSyntaxTask(
      "MortarGapHeatTransferAction", "MortarGapHeatTransfer/*", "append_mesh_generator");
  registerSyntaxTask(
      "MortarGapHeatTransferAction", "MortarGapHeatTransfer/*", "add_mortar_variable");
  registerSyntaxTask("MortarGapHeatTransferAction", "MortarGapHeatTransfer/*", "add_constraint");
  registerSyntaxTask("MortarGapHeatTransferAction", "MortarGapHeatTransfer/*", "add_user_object");
}

void
HeatTransferApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  RayTracingApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"HeatTransferApp"});
  Registry::registerActionsTo(af, {"HeatTransferApp"});
  associateSyntaxInner(s, af);
}

void
HeatTransferApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  RayTracingApp::registerObjects(factory);
  Registry::registerObjectsTo(factory, {"HeatTransferApp"});
}

void
HeatTransferApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  RayTracingApp::associateSyntax(syntax, action_factory);
  Registry::registerActionsTo(action_factory, {"HeatTransferApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
HeatTransferApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
HeatTransferApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  HeatTransferApp::registerAll(f, af, s);
}
extern "C" void
HeatTransferApp__registerApps()
{
  HeatTransferApp::registerApps();
}

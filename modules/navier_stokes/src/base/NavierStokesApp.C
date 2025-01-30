//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NavierStokesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "FluidPropertiesApp.h"
#include "HeatTransferApp.h"

InputParameters
NavierStokesApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;

  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;

  return params;
}

registerKnownLabel("NavierStokesApp");

NavierStokesApp::NavierStokesApp(InputParameters parameters) : MooseApp(parameters)
{
  NavierStokesApp::registerAll(_factory, _action_factory, _syntax);
}

NavierStokesApp::~NavierStokesApp() {}

void
NavierStokesApp::registerApps()
{
  registerApp(NavierStokesApp);

  FluidPropertiesApp::registerApps();
  HeatTransferApp::registerApps();
}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  // Physics syntax
  registerSyntax("WCNSFVFlowPhysics", "Physics/NavierStokes/Flow/*");
  registerSyntax("WCNSLinearFVFlowPhysics", "Physics/NavierStokes/FlowSegregated/*");
  registerSyntax("WCNSFVFluidHeatTransferPhysics", "Physics/NavierStokes/FluidHeatTransfer/*");
  registerSyntax("WCNSLinearFVFluidHeatTransferPhysics",
                 "Physics/NavierStokes/FluidHeatTransferSegregated/*");
  registerSyntax("WCNSFVScalarTransportPhysics", "Physics/NavierStokes/ScalarTransport/*");
  registerSyntax("WCNSLinearFVScalarTransportPhysics",
                 "Physics/NavierStokes/ScalarTransportSegregated/*");
  registerSyntax("WCNSFVTurbulencePhysics", "Physics/NavierStokes/Turbulence/*");
  registerSyntax("PNSFVSolidHeatTransferPhysics", "Physics/NavierStokes/SolidHeatTransfer/*");
  registerSyntax("WCNSFVTwoPhaseMixturePhysics", "Physics/NavierStokes/TwoPhaseMixture/*");

  // Create the Action syntax
  registerSyntax("CNSAction", "Modules/CompressibleNavierStokes");
  registerSyntax("INSAction", "Modules/IncompressibleNavierStokes");

  // Deprecated action syntax for NavierStokesFV
  registerTask("nsfv_action_deprecation_task", /*is_required=*/false);
  registerSyntax("NSFVActionDeprecation", "Modules/NavierStokesFV");
  registerSyntax("WCNSFVFlowPhysics", "Modules/NavierStokesFV");
  registerSyntax("WCNSFVFluidHeatTransferPhysics", "Modules/NavierStokesFV");
  registerSyntax("WCNSFVScalarTransportPhysics", "Modules/NavierStokesFV");
  registerSyntax("WCNSFVTurbulencePhysics", "Modules/NavierStokesFV");

  // Additional tasks to make the Physics work out
  registerTask("get_turbulence_physics", /*is_required=*/false);
  addTaskDependency("get_turbulence_physics", "init_physics");
  addTaskDependency("check_integrity_early_physics", "get_turbulence_physics");

  // add variables action
  registerTask("add_navier_stokes_variables", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_variables", "add_variable");
  addTaskDependency("add_mortar_variable", "add_navier_stokes_variables");

  // add ICs action
  registerTask("add_navier_stokes_ics", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_ics", "add_ic");
  addTaskDependency("add_constraint", "add_navier_stokes_ics");

  // add user objects action
  registerTask("add_navier_stokes_user_objects", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_user_objects", "add_user_object");
  addTaskDependency("add_distribution", "add_navier_stokes_user_objects");

  // add Kernels action
  registerTask("add_navier_stokes_kernels", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_kernels", "add_kernel");
  addTaskDependency("resolve_optional_materials", "add_navier_stokes_kernels");

  // add BCs actions
  registerTask("add_navier_stokes_bcs", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_bcs", "add_bc");
  addTaskDependency("resolve_optional_materials", "add_navier_stokes_bcs");

  // register attributes
  registerTask("ns_meta_action", /*is_required=*/true);
  addTaskDependency("ns_meta_action", "meta_action");
  addTaskDependency("dynamic_object_registration", "ns_meta_action");

  // register attributes
  registerTask("add_navier_stokes_pps", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_pps", "add_postprocessor");
  addTaskDependency("add_vector_postprocessor", "add_navier_stokes_pps");

  // register attributes
  registerTask("add_navier_stokes_materials", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_materials", "add_material");
  addTaskDependency("add_master_action_material", "add_navier_stokes_materials");

  // register tasks for copying variables
  registerTask("navier_stokes_check_copy_nodal_vars", /*is_required=*/false);
  addTaskDependency("navier_stokes_check_copy_nodal_vars", "check_copy_nodal_vars");
  addTaskDependency("setup_mesh", "navier_stokes_check_copy_nodal_vars");

  registerTask("navier_stokes_copy_nodal_vars", /*is_required=*/false);
  addTaskDependency("navier_stokes_copy_nodal_vars", "copy_nodal_vars");
  addTaskDependency("add_material", "navier_stokes_copy_nodal_vars");
}

void
NavierStokesApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FluidPropertiesApp::registerAll(f, af, s);
  HeatTransferApp::registerAll(f, af, s);
  Registry::registerObjectsTo(f, {"NavierStokesApp"});
  Registry::registerActionsTo(af, {"NavierStokesApp"});
  associateSyntaxInner(s, af);
}

void
NavierStokesApp::registerObjectDepends(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjectsDepends");
  FluidPropertiesApp::registerObjects(factory);
}

void
NavierStokesApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"NavierStokesApp"});
}

void
NavierStokesApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntaxDepends");
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
}

void
NavierStokesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"NavierStokesApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
NavierStokesApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
NavierStokesApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  NavierStokesApp::registerAll(f, af, s);
}
extern "C" void
NavierStokesApp__registerApps()
{
  NavierStokesApp::registerApps();
}

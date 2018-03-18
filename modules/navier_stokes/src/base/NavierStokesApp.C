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

// So we can register objects from the fluid_properties module.
#include "FluidPropertiesApp.h"

template <>
InputParameters
validParams<NavierStokesApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

registerKnownLabel("NavierStokesApp");

NavierStokesApp::NavierStokesApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  NavierStokesApp::registerObjectDepends(_factory);
  NavierStokesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  NavierStokesApp::associateSyntaxDepends(_syntax, _action_factory);
  NavierStokesApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  NavierStokesApp::registerExecFlags(_factory);
}

NavierStokesApp::~NavierStokesApp() {}

// External entry point for dynamic application loading
extern "C" void
NavierStokesApp__registerApps()
{
  NavierStokesApp::registerApps();
}
void
NavierStokesApp::registerApps()
{
  registerApp(NavierStokesApp);
}

void
NavierStokesApp::registerObjectDepends(Factory & factory)
{
  FluidPropertiesApp::registerObjects(factory);
}

// External entry point for dynamic object registration
extern "C" void
NavierStokesApp__registerObjects(Factory & factory)
{
  NavierStokesApp::registerObjects(factory);
}
void
NavierStokesApp::registerObjects(Factory & factory)
{
  Registry::registerObjectsTo(factory, {"NavierStokesApp"});
}

void
NavierStokesApp::associateSyntaxDepends(Syntax & syntax, ActionFactory & action_factory)
{
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
}

// External entry point for dynamic syntax association
extern "C" void
NavierStokesApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  NavierStokesApp::associateSyntax(syntax, action_factory);
}
void
NavierStokesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  Registry::registerActionsTo(action_factory, {"NavierStokesApp"});

  // Create the syntax
  registerSyntax("AddNavierStokesVariablesAction", "Modules/NavierStokes/Variables");
  registerSyntax("AddNavierStokesICsAction", "Modules/NavierStokes/ICs");
  registerSyntax("AddNavierStokesKernelsAction", "Modules/NavierStokes/Kernels");
  registerSyntax("AddNavierStokesBCsAction", "Modules/NavierStokes/BCs/*");

  // add variables action
  registerTask("add_navier_stokes_variables", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_variables", "add_variable");

  // add ICs action
  registerTask("add_navier_stokes_ics", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_ics", "add_ic");

  // add Kernels action
  registerTask("add_navier_stokes_kernels", /*is_required=*/false);
  addTaskDependency("add_navier_stokes_kernels", "add_kernel");

  // add BCs actions
  registerMooseObjectTask("add_navier_stokes_bcs", NSWeakStagnationInletBC, /*is_required=*/false);
  appendMooseObjectTask("add_navier_stokes_bcs", NSNoPenetrationBC);
  appendMooseObjectTask("add_navier_stokes_bcs", NSStaticPressureOutletBC);
  addTaskDependency("add_navier_stokes_bcs", "add_bc");
}

// External entry point for dynamic execute flag registration
extern "C" void
NavierStokesApp__registerExecFlags(Factory & factory)
{
  NavierStokesApp::registerExecFlags(factory);
}
void
NavierStokesApp::registerExecFlags(Factory & /*factory*/)
{
}

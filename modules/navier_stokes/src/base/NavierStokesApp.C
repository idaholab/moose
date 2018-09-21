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
  NavierStokesApp::registerAll(_factory, _action_factory, _syntax);
}

NavierStokesApp::~NavierStokesApp() {}

void
NavierStokesApp::registerApps()
{
  registerApp(NavierStokesApp);
}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
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

void
NavierStokesApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FluidPropertiesApp::registerAll(f, af, s);
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
  mooseDeprecated("use registerAll instead of registerExecFlags");
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

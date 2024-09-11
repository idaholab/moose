//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#ifdef AIR_FP_ENABLED
#include "AirApp.h"
#endif
#ifdef CARBON_DIOXIDE_FP_ENABLED
#include "CarbonDioxideApp.h"
#endif
#ifdef HELIUM_FP_ENABLED
#include "HeliumApp.h"
#endif
#ifdef NITROGEN_FP_ENABLED
#include "NitrogenApp.h"
#endif
#ifdef POTASSIUM_FP_ENABLED
#include "PotassiumApp.h"
#endif
#ifdef SODIUM_FP_ENABLED
#include "SodiumApp.h"
#endif

InputParameters
FluidPropertiesApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  params.set<bool>("use_legacy_material_output") = false;
  params.set<bool>("use_legacy_initial_residual_evaluation_behavior") = false;
  return params;
}

registerKnownLabel("FluidPropertiesApp");

FluidPropertiesApp::FluidPropertiesApp(InputParameters parameters) : MooseApp(parameters)
{
  FluidPropertiesApp::registerAll(_factory, _action_factory, _syntax);
}

FluidPropertiesApp::~FluidPropertiesApp() {}

void
FluidPropertiesApp::registerApps()
{
  registerApp(FluidPropertiesApp);
}

static void
associateSyntaxInner(Syntax & syntax, ActionFactory & /*action_factory*/)
{
  registerSyntaxTask(
      "AddFluidPropertiesDeprecatedAction", "Modules/FluidProperties/*", "add_fluid_properties");
  registerSyntaxTask("AddFluidPropertiesAction", "FluidProperties/*", "add_fluid_properties");
  registerMooseObjectTask("add_fluid_properties", FluidProperties, false);
  registerMooseObjectTask("add_fp_output", Output, false);

  // Fluid properties depend on variables
  syntax.addDependency("add_aux_variable", "add_fluid_properties");
  syntax.addDependency("add_variable", "add_fluid_properties");
  syntax.addDependency("add_elemental_field_variable", "add_fluid_properties");
  syntax.addDependency("add_external_aux_variables", "add_fluid_properties");
  syntax.addDependency("add_fp_output", "add_output");
  syntax.addDependency("add_postprocessor", "add_fp_output");

  syntax.registerActionSyntax("AddFluidPropertiesInterrogatorAction",
                              "FluidPropertiesInterrogator");
}

void
FluidPropertiesApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
#ifdef AIR_FP_ENABLED
  AirApp::registerAll(f, af, s);
#endif
#ifdef CARBON_DIOXIDE_FP_ENABLED
  CarbonDioxideApp::registerAll(f, af, s);
#endif
#ifdef HELIUM_FP_ENABLED
  HeliumApp::registerAll(f, af, s);
#endif
#ifdef NITROGEN_FP_ENABLED
  NitrogenApp::registerAll(f, af, s);
#endif
#ifdef POTASSIUM_FP_ENABLED
  PotassiumApp::registerAll(f, af, s);
#endif
#ifdef SODIUM_FP_ENABLED
  SodiumApp::registerAll(f, af, s);
#endif

  Registry::registerObjectsTo(f, {"FluidPropertiesApp"});
  Registry::registerActionsTo(af, {"FluidPropertiesApp"});
  associateSyntaxInner(s, af);
}

void
FluidPropertiesApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  Registry::registerObjectsTo(factory, {"FluidPropertiesApp"});
}

void
FluidPropertiesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  Registry::registerActionsTo(action_factory, {"FluidPropertiesApp"});
  associateSyntaxInner(syntax, action_factory);
}

void
FluidPropertiesApp::registerExecFlags(Factory & /*factory*/)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
}

extern "C" void
FluidPropertiesApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  FluidPropertiesApp::registerAll(f, af, s);
}
extern "C" void
FluidPropertiesApp__registerApps()
{
  FluidPropertiesApp::registerApps();
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

FluidPropertiesApp::FluidPropertiesApp(const InputParameters & parameters) : MooseApp(parameters)
{
  FluidPropertiesApp::registerAll(_factory, _action_factory, _syntax);
}

FluidPropertiesApp::~FluidPropertiesApp() {}

void
FluidPropertiesApp::registerApps()
{
  const std::string doc = "Saline thermophysical fluid properties ";
#ifdef SALINE_ENABLED
  addCapability("saline", true, doc + "are available.");
#else
  addCapability("saline", false, doc + "are not available.");
#endif

  registerApp(FluidPropertiesApp);
#ifdef AIR_FP_ENABLED
  registerApp(AirApp);
#endif
#ifdef CARBON_DIOXIDE_FP_ENABLED
  registerApp(CarbonDioxideApp);
#endif
#ifdef HELIUM_FP_ENABLED
  registerApp(HeliumApp);
#endif
#ifdef NITROGEN_FP_ENABLED
  registerApp(NitrogenApp);
#endif
#ifdef POTASSIUM_FP_ENABLED
  registerApp(PotassiumApp);
#endif
#ifdef SODIUM_FP_ENABLED
  registerApp(SodiumApp);
#endif
}

void
FluidPropertiesApp::registerAll(Factory & f, ActionFactory & af, Syntax & syntax)
{
#ifdef AIR_FP_ENABLED
  AirApp::registerAll(f, af, syntax);
#endif
#ifdef CARBON_DIOXIDE_FP_ENABLED
  CarbonDioxideApp::registerAll(f, af, syntax);
#endif
#ifdef HELIUM_FP_ENABLED
  HeliumApp::registerAll(f, af, syntax);
#endif
#ifdef NITROGEN_FP_ENABLED
  NitrogenApp::registerAll(f, af, syntax);
#endif
#ifdef POTASSIUM_FP_ENABLED
  PotassiumApp::registerAll(f, af, syntax);
#endif
#ifdef SODIUM_FP_ENABLED
  SodiumApp::registerAll(f, af, syntax);
#endif

  Registry::registerObjectsTo(f, {"FluidPropertiesApp"});
  Registry::registerActionsTo(af, {"FluidPropertiesApp"});

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

/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "FluidPropertiesApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "FluidPropertiesMaterial.h"
#include "FluidPropertiesMaterialPT.h"
#include "MultiComponentFluidPropertiesMaterialPT.h"

#include "IdealGasFluidProperties.h"
#include "IdealGasFluidPropertiesPT.h"
#include "StiffenedGasFluidProperties.h"
#include "MethaneFluidProperties.h"
#include "Water97FluidProperties.h"
#include "CO2FluidProperties.h"
#include "NaClFluidProperties.h"
#include "BrineFluidProperties.h"
#include "SimpleFluidProperties.h"

#include "SpecificEnthalpyAux.h"
#include "StagnationPressureAux.h"
#include "StagnationTemperatureAux.h"

#include "AddFluidPropertiesAction.h"

template <>
InputParameters
validParams<FluidPropertiesApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

FluidPropertiesApp::FluidPropertiesApp(InputParameters parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  FluidPropertiesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  FluidPropertiesApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags();
  FluidPropertiesApp::registerExecFlags();
}

FluidPropertiesApp::~FluidPropertiesApp() {}

// External entry point for dynamic application loading
extern "C" void
FluidPropertiesApp__registerApps()
{
  FluidPropertiesApp::registerApps();
}

void
FluidPropertiesApp::registerApps()
{
  registerApp(FluidPropertiesApp);
}

// External entry point for dynamic object registration
extern "C" void
FluidPropertiesApp__registerObjects(Factory & factory)
{
  FluidPropertiesApp::registerObjects(factory);
}

void
FluidPropertiesApp::registerObjects(Factory & factory)
{
  registerMaterial(FluidPropertiesMaterial);
  registerMaterial(FluidPropertiesMaterialPT);
  registerMaterial(MultiComponentFluidPropertiesMaterialPT);

  registerUserObject(IdealGasFluidProperties);
  registerUserObject(IdealGasFluidPropertiesPT);
  registerUserObject(StiffenedGasFluidProperties);
  registerUserObject(MethaneFluidProperties);
  registerUserObject(Water97FluidProperties);
  registerUserObject(CO2FluidProperties);
  registerUserObject(NaClFluidProperties);
  registerUserObject(BrineFluidProperties);
  registerUserObject(SimpleFluidProperties);

  registerAuxKernel(SpecificEnthalpyAux);
  registerAuxKernel(StagnationPressureAux);
  registerAuxKernel(StagnationTemperatureAux);
}

// External entry point for dynamic syntax association
extern "C" void
FluidPropertiesApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
}

void
FluidPropertiesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  registerSyntaxTask(
      "AddFluidPropertiesAction", "Modules/FluidProperties/*", "add_fluid_properties");

  registerMooseObjectTask("add_fluid_properties", FluidProperties, false);

  syntax.addDependency("add_fluid_properties", "init_displaced_problem");

  registerAction(AddFluidPropertiesAction, "add_fluid_properties");
}

void
FluidPropertiesApp::registerExecFlags()
{
}

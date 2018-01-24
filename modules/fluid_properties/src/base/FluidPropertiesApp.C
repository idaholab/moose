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

#include "FluidPropertiesMaterial.h"
#include "FluidPropertiesMaterialPT.h"

#include "IdealGasFluidProperties.h"
#include "IdealGasFluidPropertiesPT.h"
#include "StiffenedGasFluidProperties.h"
#include "MethaneFluidProperties.h"
#include "Water97FluidProperties.h"
#include "CO2FluidProperties.h"
#include "NaClFluidProperties.h"
#include "BrineFluidProperties.h"
#include "SimpleFluidProperties.h"
#include "TabulatedFluidProperties.h"
#include "SodiumProperties.h"

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

  Moose::registerExecFlags(_factory);
  FluidPropertiesApp::registerExecFlags(_factory);
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

  registerUserObject(IdealGasFluidProperties);
  registerUserObject(IdealGasFluidPropertiesPT);
  registerUserObject(StiffenedGasFluidProperties);
  registerUserObject(MethaneFluidProperties);
  registerUserObject(Water97FluidProperties);
  registerUserObject(CO2FluidProperties);
  registerUserObject(NaClFluidProperties);
  registerUserObject(BrineFluidProperties);
  registerUserObject(SimpleFluidProperties);
  registerUserObject(TabulatedFluidProperties);
  registerUserObject(SodiumProperties);
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

// External entry point for dynamic execute flag registration
extern "C" void
FluidPropertiesApp__registerExecFlags(Factory & factory)
{
  FluidPropertiesApp::registerExecFlags(factory);
}
void
FluidPropertiesApp::registerExecFlags(Factory & /*factory*/)
{
}

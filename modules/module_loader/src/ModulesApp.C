//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ModulesApp.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#ifdef CHEMICAL_REACTIONS_ENABLED
#include "ChemicalReactionsApp.h"
#endif
#ifdef CONTACT_ENABLED
#include "ContactApp.h"
#endif
#ifdef FLUID_PROPERTIES_ENABLED
#include "FluidPropertiesApp.h"
#endif
#ifdef HEAT_CONDUCTION_ENABLED
#include "HeatConductionApp.h"
#endif
#ifdef LEVEL_SET_ENABLED
#include "LevelSetApp.h"
#endif
#ifdef MISC_ENABLED
#include "MiscApp.h"
#endif
#ifdef NAVIER_STOKES_ENABLED
#include "NavierStokesApp.h"
#endif
#ifdef PHASE_FIELD_ENABLED
#include "PhaseFieldApp.h"
#endif
#ifdef POROUS_FLOW_ENABLED
#include "PorousFlowApp.h"
#endif
#ifdef RDG_ENABLED
#include "RdgApp.h"
#endif
#ifdef RICHARDS_ENABLED
#include "RichardsApp.h"
#endif
#ifdef SOLID_MECHANICS_ENABLED
#include "SolidMechanicsApp.h"
#endif
#ifdef STOCHASTIC_TOOLS_ENABLED
#include "StochasticToolsApp.h"
#endif
#ifdef TENSOR_MECHANICS_ENABLED
#include "TensorMechanicsApp.h"
#endif
#ifdef WATER_STEAM_EOS_ENABLED
#include "WaterSteamEOSApp.h"
#endif
#ifdef XFEM_ENABLED
#include "XFEMApp.h"
#endif

///@{
/**
 * Dummy methods to clear unused parameter warnings from compiler
 */
void
clearUnusedWarnings(Factory & /*factory*/)
{
}
void
clearUnusedWarnings(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
///@}

template <>
InputParameters
validParams<ModulesApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

ModulesApp::ModulesApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  ModulesApp::registerExecFlags(_factory);
}

ModulesApp::~ModulesApp() {}

// External entry point for dynamic application loading
extern "C" void
ModulesApp__registerApps()
{
  ModulesApp::registerApps();
}
void
ModulesApp::registerApps()
{
  registerApp(ModulesApp);
}

// External entry point for dynamic object registration
extern "C" void
ModulesApp__registerObjects(Factory & factory)
{
  ModulesApp::registerObjects(factory);
}
void
ModulesApp::registerObjects(Factory & factory)
{
#ifdef CHEMICAL_REACTIONS_ENABLED
  ChemicalReactionsApp::registerObjects(factory);
#endif

#ifdef CONTACT_ENABLED
  ContactApp::registerObjects(factory);
#endif

#ifdef FLUID_PROPERTIES_ENABLED
  FluidPropertiesApp::registerObjects(factory);
#endif

#ifdef HEAT_CONDUCTION_ENABLED
  HeatConductionApp::registerObjects(factory);
#endif

#ifdef LEVEL_SET_ENABLED
  LevelSetApp::registerObjects(factory);
#endif

#ifdef MISC_ENABLED
  MiscApp::registerObjects(factory);
#endif

#ifdef NAVIER_STOKES_ENABLED
  NavierStokesApp::registerObjects(factory);
#endif

#ifdef PHASE_FIELD_ENABLED
  PhaseFieldApp::registerObjects(factory);
#endif

#ifdef POROUS_FLOW_ENABLED
  PorousFlowApp::registerObjects(factory);
#endif

#ifdef RDG_ENABLED
  RdgApp::registerObjects(factory);
#endif

#ifdef RICHARDS_ENABLED
  RichardsApp::registerObjects(factory);
#endif

#ifdef SOLID_MECHANICS_ENABLED
  SolidMechanicsApp::registerObjects(factory);
#endif

#ifdef STOCHASTIC_TOOLS_ENABLED
  StochasticToolsApp::registerObjects(factory);
#endif

#ifdef TENSOR_MECHANICS_ENABLED
  TensorMechanicsApp::registerObjects(factory);
#endif

#ifdef WATER_STEAM_EOS_ENABLED
  WaterSteamEOSApp::registerObjects(factory);
#endif

#ifdef XFEM_ENABLED
  XFEMApp::registerObjects(factory);
#endif

  clearUnusedWarnings(factory);
}

// External entry point for dynamic syntax association
extern "C" void
ModulesApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ModulesApp::associateSyntax(syntax, action_factory);
}
void
ModulesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
#ifdef CHEMICAL_REACTIONS_ENABLED
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef CONTACT_ENABLED
  ContactApp::associateSyntax(syntax, action_factory);
#endif

#ifdef FLUID_PROPERTIES_ENABLED
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
#endif

#ifdef HEAT_CONDUCTION_ENABLED
  HeatConductionApp::associateSyntax(syntax, action_factory);
#endif

#ifdef LEVEL_SET_ENABLED
  LevelSetApp::associateSyntax(syntax, action_factory);
#endif

#ifdef MISC_ENABLED
  MiscApp::associateSyntax(syntax, action_factory);
#endif

#ifdef NAVIER_STOKES_ENABLED
  NavierStokesApp::associateSyntax(syntax, action_factory);
#endif

#ifdef PHASE_FIELD_ENABLED
  PhaseFieldApp::associateSyntax(syntax, action_factory);
#endif

#ifdef POROUS_FLOW_ENABLED
  PorousFlowApp::associateSyntax(syntax, action_factory);
#endif

#ifdef RDG_ENABLED
  RdgApp::associateSyntax(syntax, action_factory);
#endif

#ifdef RICHARDS_ENABLED
  RichardsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef SOLID_MECHANICS_ENABLED
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef STOCHASTIC_TOOLS_ENABLED
  StochasticToolsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef TENSOR_MECHANICS_ENABLED
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef WATER_STEAM_EOS_ENABLED
  WaterSteamEOSApp::associateSyntax(syntax, action_factory);
#endif

#ifdef XFEM_ENABLED
  XFEMApp::associateSyntax(syntax, action_factory);
#endif

  clearUnusedWarnings(syntax, action_factory);
}

// External entry point for dynamic object registration
extern "C" void
ModulesApp__registerExecFlags(Factory & factory)
{
  ModulesApp::registerExecFlags(factory);
}
void
ModulesApp::registerExecFlags(Factory & factory)
{
#ifdef CHEMICAL_REACTIONS_ENABLED
  ChemicalReactionsApp::registerExecFlags(factory);
#endif

#ifdef CONTACT_ENABLED
  ContactApp::registerExecFlags(factory);
#endif

#ifdef FLUID_PROPERTIES_ENABLED
  FluidPropertiesApp::registerExecFlags(factory);
#endif

#ifdef HEAT_CONDUCTION_ENABLED
  HeatConductionApp::registerExecFlags(factory);
#endif

#ifdef LEVEL_SET_ENABLED
  LevelSetApp::registerExecFlags(factory);
#endif

#ifdef MISC_ENABLED
  MiscApp::registerExecFlags(factory);
#endif

#ifdef NAVIER_STOKES_ENABLED
  NavierStokesApp::registerExecFlags(factory);
#endif

#ifdef PHASE_FIELD_ENABLED
  PhaseFieldApp::registerExecFlags(factory);
#endif

#ifdef POROUS_FLOW_ENABLED
  PorousFlowApp::registerExecFlags(factory);
#endif

#ifdef RDG_ENABLED
  RdgApp::registerExecFlags(factory);
#endif

#ifdef RICHARDS_ENABLED
  RichardsApp::registerExecFlags(factory);
#endif

#ifdef SOLID_MECHANICS_ENABLED
  SolidMechanicsApp::registerExecFlags(factory);
#endif

#ifdef STOCHASTIC_TOOLS_ENABLED
  StochasticToolsApp::registerExecFlags(factory);
#endif

#ifdef TENSOR_MECHANICS_ENABLED
  TensorMechanicsApp::registerExecFlags(factory);
#endif

#ifdef WATER_STEAM_EOS_ENABLED
  WaterSteamEOSApp::registerExecFlags(factory);
#endif

#ifdef XFEM_ENABLED
  XFEMApp::registerExecFlags(factory);
#endif

  clearUnusedWarnings(factory);
}

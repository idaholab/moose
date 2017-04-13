/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
#ifdef LINEAR_ELASTICITY_ENABLED
#include "LinearElasticityApp.h"
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
#ifdef RICHARDS_ENABLED
#include "RichardsApp.h"
#endif
#ifdef SOLID_MECHANICS_ENABLED
#include "SolidMechanicsApp.h"
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

#ifdef LINEAR_ELASTICITY_ENABLED
  LinearElasticityApp::registerObjects(factory);
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

#ifdef RICHARDS_ENABLED
  RichardsApp::registerObjects(factory);
#endif

#ifdef SOLID_MECHANICS_ENABLED
  SolidMechanicsApp::registerObjects(factory);
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

#ifdef LINEAR_ELASTICITY_ENABLED
  LinearElasticityApp::associateSyntax(syntax, action_factory);
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

#ifdef RICHARDS_ENABLED
  RichardsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef SOLID_MECHANICS_ENABLED
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
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
}

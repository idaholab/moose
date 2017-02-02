/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "CombinedApp.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "ChemicalReactionsApp.h"
#include "ContactApp.h"
#include "FluidPropertiesApp.h"
#include "HeatConductionApp.h"
#include "MiscApp.h"
#include "NavierStokesApp.h"
#include "PhaseFieldApp.h"
#include "RichardsApp.h"
#include "SolidMechanicsApp.h"
#include "StochasticToolsApp.h"
#include "TensorMechanicsApp.h"
#include "WaterSteamEOSApp.h"
#include "XFEMApp.h"
#include "PorousFlowApp.h"
#include "RdgApp.h"
#include "LevelSetApp.h"

template<>
InputParameters validParams<CombinedApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

CombinedApp::CombinedApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  CombinedApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  CombinedApp::associateSyntax(_syntax, _action_factory);
}

CombinedApp::~CombinedApp()
{
}

// External entry point for dynamic application loading
extern "C" void CombinedApp__registerApps() { CombinedApp::registerApps(); }
void
CombinedApp::registerApps()
{
  registerApp(CombinedApp);
}

// External entry point for dynamic object registration
extern "C" void CombinedApp__registerObjects(Factory & factory) { CombinedApp::registerObjects(factory); }
void
CombinedApp::registerObjects(Factory & factory)
{
  ChemicalReactionsApp::registerObjects(factory);
  ContactApp::registerObjects(factory);
  FluidPropertiesApp::registerObjects(factory);
  HeatConductionApp::registerObjects(factory);
  MiscApp::registerObjects(factory);
  NavierStokesApp::registerObjects(factory);
  PhaseFieldApp::registerObjects(factory);
  RichardsApp::registerObjects(factory);
  SolidMechanicsApp::registerObjects(factory);
  StochasticToolsApp::registerObjects(factory);
  TensorMechanicsApp::registerObjects(factory);
  WaterSteamEOSApp::registerObjects(factory);
  XFEMApp::registerObjects(factory);
  PorousFlowApp::registerObjects(factory);
  RdgApp::registerObjects(factory);
  LevelSetApp::registerObjects(factory);
}

// External entry point for dynamic syntax association
extern "C" void CombinedApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { CombinedApp::associateSyntax(syntax, action_factory); }
void
CombinedApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
  ContactApp::associateSyntax(syntax, action_factory);
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
  HeatConductionApp::associateSyntax(syntax, action_factory);
  MiscApp::associateSyntax(syntax, action_factory);
  NavierStokesApp::associateSyntax(syntax, action_factory);
  PhaseFieldApp::associateSyntax(syntax, action_factory);
  RichardsApp::associateSyntax(syntax, action_factory);
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
  StochasticToolsApp::associateSyntax(syntax, action_factory);
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
  WaterSteamEOSApp::associateSyntax(syntax, action_factory);
  XFEMApp::associateSyntax(syntax, action_factory);
  PorousFlowApp::associateSyntax(syntax, action_factory);
  RdgApp::associateSyntax(syntax, action_factory);
  LevelSetApp::associateSyntax(syntax, action_factory);
}

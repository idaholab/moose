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

#include "ChemicalReactionsApp.h"
#include "ContactApp.h"
#include "HeatConductionApp.h"
#include "LinearElasticityApp.h"
#include "MiscApp.h"
#include "NavierStokesApp.h"
#include "PhaseFieldApp.h"
#include "RichardsApp.h"
#include "SolidMechanicsApp.h"
#include "TensorMechanicsApp.h"
#include "WaterSteamEOSApp.h"
#include "XFEMApp.h"
#include "PorousFlowApp.h"

template<>
InputParameters validParams<ModulesApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

ModulesApp::ModulesApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
}

ModulesApp::~ModulesApp()
{
}

// External entry point for dynamic application loading
extern "C" void ModulesApp__registerApps() { ModulesApp::registerApps(); }
void
ModulesApp::registerApps()
{
  registerApp(ModulesApp);
}

// External entry point for dynamic object registration
extern "C" void ModulesApp__registerObjects(Factory & factory) { ModulesApp::registerObjects(factory); }
void
ModulesApp::registerObjects(Factory & factory)
{
  ChemicalReactionsApp::registerObjects(factory);
  ContactApp::registerObjects(factory);
  HeatConductionApp::registerObjects(factory);
  LinearElasticityApp::registerObjects(factory);
  MiscApp::registerObjects(factory);
  NavierStokesApp::registerObjects(factory);
  PhaseFieldApp::registerObjects(factory);
  RichardsApp::registerObjects(factory);
  SolidMechanicsApp::registerObjects(factory);
  TensorMechanicsApp::registerObjects(factory);
  WaterSteamEOSApp::registerObjects(factory);
  XFEMApp::registerObjects(factory);
  PorousFlowApp::registerObjects(factory);
}

// External entry point for dynamic syntax association
extern "C" void ModulesApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { ModulesApp::associateSyntax(syntax, action_factory); }
void
ModulesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
  ContactApp::associateSyntax(syntax, action_factory);
  HeatConductionApp::associateSyntax(syntax, action_factory);
  LinearElasticityApp::associateSyntax(syntax, action_factory);
  MiscApp::associateSyntax(syntax, action_factory);
  NavierStokesApp::associateSyntax(syntax, action_factory);
  PhaseFieldApp::associateSyntax(syntax, action_factory);
  RichardsApp::associateSyntax(syntax, action_factory);
  SolidMechanicsApp::associateSyntax(syntax, action_factory);
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
  WaterSteamEOSApp::associateSyntax(syntax, action_factory);
  XFEMApp::associateSyntax(syntax, action_factory);
  PorousFlowApp::associateSyntax(syntax, action_factory);
}

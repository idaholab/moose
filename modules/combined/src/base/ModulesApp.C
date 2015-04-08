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

/************************************************************
 * New Module Step 1.                                       *
 *         Add include for new modules here                 *
 * #include "ModuleNameApp.h"                               *
 ***********************************************************/
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

template<>
InputParameters validParams<ModulesApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = true;
  params.set<bool>("use_legacy_uo_aux_computation") = false;

  return params;
}

ModulesApp::ModulesApp(const std::string & name, InputParameters parameters) :
    MooseApp(name, parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
}

ModulesApp::~ModulesApp()
{
}

void
ModulesApp::registerApps()
{
  registerApp(ModulesApp);
}

void
ModulesApp::registerObjects(Factory & factory)
{
  /************************************************************
   * New Module Step 2.                                       *
   *                Register module objects here              *
   * ModuleNameApp::registerObjects(factory);                 *
   ***********************************************************/
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
}

void
ModulesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  /************************************************************
   * New Module Step 3.                                       *
   *                Associate syntax here                     *
   * ModuleNameApp::associateSyntax(syntax, action_factory);  *
   ***********************************************************/
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
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CombinedApp.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "ChemicalReactionsApp.h"
#include "ContactApp.h"
#include "FluidPropertiesApp.h"
#include "FunctionalExpansionToolsApp.h"
#include "GeochemistryApp.h"
#include "HeatConductionApp.h"
#include "LevelSetApp.h"
#include "MiscApp.h"
#include "NavierStokesApp.h"
#include "PhaseFieldApp.h"
#include "PorousFlowApp.h"
#include "RdgApp.h"
#include "RichardsApp.h"
#include "StochasticToolsApp.h"
#include "PeridynamicsApp.h"
#include "TensorMechanicsApp.h"
#include "XFEMApp.h"
#include "ExternalPetscSolverApp.h"

InputParameters
CombinedApp::validParams()
{
  InputParameters params = MooseApp::validParams();

  params.set<bool>("automatic_automatic_scaling") = false;

  // Do not use legacy DirichletBC, that is, set DirichletBC default for preset = true
  params.set<bool>("use_legacy_dirichlet_bc") = false;

  params.set<bool>("use_legacy_material_output") = false;

  return params;
}

registerKnownLabel("CombinedApp");

CombinedApp::CombinedApp(const InputParameters & parameters) : MooseApp(parameters)
{
  CombinedApp::registerAll(_factory, _action_factory, _syntax);
}

CombinedApp::~CombinedApp() {}

void
CombinedApp::registerApps()
{
  registerApp(CombinedApp);
}

void
CombinedApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  Registry::registerObjectsTo(f, {"CombinedApp"});
  Registry::registerActionsTo(af, {"CombinedApp"});

  ChemicalReactionsApp::registerAll(f, af, s);
  ContactApp::registerAll(f, af, s);
  FluidPropertiesApp::registerAll(f, af, s);
  FunctionalExpansionToolsApp::registerAll(f, af, s);
  GeochemistryApp::registerAll(f, af, s);
  HeatConductionApp::registerAll(f, af, s);
  LevelSetApp::registerAll(f, af, s);
  MiscApp::registerAll(f, af, s);
  NavierStokesApp::registerAll(f, af, s);
  PhaseFieldApp::registerAll(f, af, s);
  PorousFlowApp::registerAll(f, af, s);
  RdgApp::registerAll(f, af, s);
  RichardsApp::registerAll(f, af, s);
  StochasticToolsApp::registerAll(f, af, s);
  PeridynamicsApp::registerAll(f, af, s);
  TensorMechanicsApp::registerAll(f, af, s);
  XFEMApp::registerAll(f, af, s);
  ExternalPetscSolverApp::registerAll(f, af, s);
}

void
CombinedApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerObjects");
  ChemicalReactionsApp::registerObjects(factory);
  ContactApp::registerObjects(factory);
  FluidPropertiesApp::registerObjects(factory);
  FunctionalExpansionToolsApp::registerObjects(factory);
  HeatConductionApp::registerObjects(factory);
  LevelSetApp::registerObjects(factory);
  MiscApp::registerObjects(factory);
  NavierStokesApp::registerObjects(factory);
  PhaseFieldApp::registerObjects(factory);
  PorousFlowApp::registerObjects(factory);
  RdgApp::registerObjects(factory);
  RichardsApp::registerObjects(factory);
  StochasticToolsApp::registerObjects(factory);
  PeridynamicsApp::registerObjects(factory);
  TensorMechanicsApp::registerObjects(factory);
  XFEMApp::registerObjects(factory);
}

void
CombinedApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use registerAll instead of associateSyntax");
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
  ContactApp::associateSyntax(syntax, action_factory);
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
  FunctionalExpansionToolsApp::associateSyntax(syntax, action_factory);
  HeatConductionApp::associateSyntax(syntax, action_factory);
  LevelSetApp::associateSyntax(syntax, action_factory);
  MiscApp::associateSyntax(syntax, action_factory);
  NavierStokesApp::associateSyntax(syntax, action_factory);
  PhaseFieldApp::associateSyntax(syntax, action_factory);
  PorousFlowApp::associateSyntax(syntax, action_factory);
  RdgApp::associateSyntax(syntax, action_factory);
  RichardsApp::associateSyntax(syntax, action_factory);
  StochasticToolsApp::associateSyntax(syntax, action_factory);
  PeridynamicsApp::associateSyntax(syntax, action_factory);
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
  XFEMApp::associateSyntax(syntax, action_factory);
}

void
CombinedApp::registerExecFlags(Factory & factory)
{
  mooseDeprecated("use registerAll instead of registerExecFlags");
  ChemicalReactionsApp::registerExecFlags(factory);
  ContactApp::registerExecFlags(factory);
  FluidPropertiesApp::registerExecFlags(factory);
  HeatConductionApp::registerExecFlags(factory);
  MiscApp::registerExecFlags(factory);
  NavierStokesApp::registerExecFlags(factory);
  PhaseFieldApp::registerExecFlags(factory);
  RichardsApp::registerExecFlags(factory);
  StochasticToolsApp::registerExecFlags(factory);
  PeridynamicsApp::registerExecFlags(factory);
  TensorMechanicsApp::registerExecFlags(factory);
  XFEMApp::registerExecFlags(factory);
  PorousFlowApp::registerExecFlags(factory);
  RdgApp::registerExecFlags(factory);
  LevelSetApp::registerExecFlags(factory);
}

extern "C" void
CombinedApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  CombinedApp::registerAll(f, af, s);
}
extern "C" void
CombinedApp__registerApps()
{
  CombinedApp::registerApps();
}

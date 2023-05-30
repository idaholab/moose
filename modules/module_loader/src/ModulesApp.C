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
#ifdef ELECTROMAGNETICS_ENABLED
#include "ElectromagneticsApp.h"
#endif
#ifdef FLUID_PROPERTIES_ENABLED
#include "FluidPropertiesApp.h"
#endif
#ifdef FSI_ENABLED
#include "FsiApp.h"
#endif
#ifdef FUNCTIONAL_EXPANSION_TOOLS_ENABLED
#include "FunctionalExpansionToolsApp.h"
#endif
#ifdef GEOCHEMISTRY_ENABLED
#include "GeochemistryApp.h"
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
#ifdef OPTIMIZATION_ENABLED
#include "OptimizationApp.h"
#endif
#ifdef PERIDYNAMICS_ENABLED
#include "PeridynamicsApp.h"
#endif
#ifdef PHASE_FIELD_ENABLED
#include "PhaseFieldApp.h"
#endif
#ifdef POROUS_FLOW_ENABLED
#include "PorousFlowApp.h"
#endif
#ifdef RAY_TRACING_ENABLED
#include "RayTracingApp.h"
#endif
#ifdef RDG_ENABLED
#include "RdgApp.h"
#endif
#ifdef REACTOR_ENABLED
#include "ReactorApp.h"
#endif
#ifdef RICHARDS_ENABLED
#include "RichardsApp.h"
#endif
#ifdef SCALAR_TRANSPORT_ENABLED
#include "ScalarTransportApp.h"
#endif
#ifdef SOLID_PROPERTIES_ENABLED
#include "SolidPropertiesApp.h"
#endif
#ifdef STOCHASTIC_TOOLS_ENABLED
#include "StochasticToolsApp.h"
#endif
#ifdef TENSOR_MECHANICS_ENABLED
#include "TensorMechanicsApp.h"
#endif
#ifdef THERMAL_HYDRAULICS_ENABLED
#include "ThermalHydraulicsApp.h"
#endif
#ifdef XFEM_ENABLED
#include "XFEMApp.h"
#endif
#ifdef EXTERNAL_PETSC_SOLVER_ENABLED
#include "ExternalPetscSolverApp.h"
#endif

InputParameters
ModulesApp::validParams()
{
  InputParameters params = MooseApp::validParams();
  return params;
}

ModulesApp::ModulesApp(const InputParameters & parameters) : MooseApp(parameters)
{
  ModulesApp::registerAll(_factory, _action_factory, _syntax);
}

ModulesApp::~ModulesApp() {}

void
ModulesApp::registerApps()
{
  registerApp(ModulesApp);
}

void
ModulesApp::registerObjects(Factory & factory)
{
  mooseDeprecated("use ModulesApp::registerAll instead of ModulesApp::registerObjects");
#ifdef CHEMICAL_REACTIONS_ENABLED
  ChemicalReactionsApp::registerObjects(factory);
#endif

#ifdef CONTACT_ENABLED
  ContactApp::registerObjects(factory);
#endif

#ifdef ELECTROMAGNETICS_ENABLED
  ElectromagneticsApp::registerObjects(factory);
#endif

#ifdef FLUID_PROPERTIES_ENABLED
  FluidPropertiesApp::registerObjects(factory);
#endif

#ifdef FUNCTIONAL_EXPANSION_TOOLS_ENABLED
  FunctionalExpansionToolsApp::registerObjects(factory);
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

#ifdef PERIDYNAMICS_ENABLED
  PeridynamicsApp::registerObjects(factory);
#endif

#ifdef PHASE_FIELD_ENABLED
  PhaseFieldApp::registerObjects(factory);
#endif

#ifdef POROUS_FLOW_ENABLED
  PorousFlowApp::registerObjects(factory);
#endif

#ifdef RAY_TRACING_ENABLED
  RayTracingApp::registerObjects(factory);
#endif

#ifdef RDG_ENABLED
  RdgApp::registerObjects(factory);
#endif

#ifdef REACTOR_ENABLED
  ReactorApp::registerObjects(factory);
#endif

#ifdef RICHARDS_ENABLED
  RichardsApp::registerObjects(factory);
#endif

#ifdef SOLID_PROPERTIES_ENABLED
  SolidPropertiesApp::registerObjects(factory);
#endif

#ifdef STOCHASTIC_TOOLS_ENABLED
  StochasticToolsApp::registerObjects(factory);
#endif

#ifdef TENSOR_MECHANICS_ENABLED
  TensorMechanicsApp::registerObjects(factory);
#endif

#ifdef THERMAL_HYDRAULICS_ENABLED
  ThermalHydraulicsApp::registerObjects(factory);
#endif

#ifdef XFEM_ENABLED
  XFEMApp::registerObjects(factory);
#endif

  libmesh_ignore(factory);
}

void
ModulesApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  mooseDeprecated("use ModulesApp::registerAll instead of ModulesApp::associateSyntax");
#ifdef CHEMICAL_REACTIONS_ENABLED
  ChemicalReactionsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef CONTACT_ENABLED
  ContactApp::associateSyntax(syntax, action_factory);
#endif

#ifdef ELECTROMAGNETICS_ENABLED
  ElectromagneticsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef FLUID_PROPERTIES_ENABLED
  FluidPropertiesApp::associateSyntax(syntax, action_factory);
#endif

#ifdef FUNCTIONAL_EXPANSION_TOOLS_ENABLED
  FunctionalExpansionToolsApp::associateSyntax(syntax, action_factory);
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

#ifdef PERIDYNAMICS_ENABLED
  PeridynamicsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef PHASE_FIELD_ENABLED
  PhaseFieldApp::associateSyntax(syntax, action_factory);
#endif

#ifdef POROUS_FLOW_ENABLED
  PorousFlowApp::associateSyntax(syntax, action_factory);
#endif

#ifdef RAY_TRACING_ENABLED
  RayTracingApp::associateSyntax(syntax, action_factory);
#endif

#ifdef RDG_ENABLED
  RdgApp::associateSyntax(syntax, action_factory);
#endif

#ifdef REACTOR_ENABLED
  ReactorApp::associateSyntax(syntax, action_factory);
#endif

#ifdef RICHARDS_ENABLED
  RichardsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef SOLID_PROPERTIES_ENABLED
  SolidPropertiesApp::associateSyntax(syntax, action_factory);
#endif

#ifdef STOCHASTIC_TOOLS_ENABLED
  StochasticToolsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef TENSOR_MECHANICS_ENABLED
  TensorMechanicsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef THERMAL_HYDRAULICS_ENABLED
  ThermalHydraulicsApp::associateSyntax(syntax, action_factory);
#endif

#ifdef XFEM_ENABLED
  XFEMApp::associateSyntax(syntax, action_factory);
#endif

  libmesh_ignore(syntax, action_factory);
}

void
ModulesApp::registerExecFlags(Factory & factory)
{
  mooseDeprecated("Do not use registerExecFlags, apps no longer require flag registration");
#ifdef CHEMICAL_REACTIONS_ENABLED
  ChemicalReactionsApp::registerExecFlags(factory);
#endif

#ifdef CONTACT_ENABLED
  ContactApp::registerExecFlags(factory);
#endif

#ifdef ELECTROMAGNETICS_ENABLED
  ElectromagneticsApp::registerExecFlags(factory);
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

#ifdef PERIDYNAMICS_ENABLED
  PeridynamicsApp::registerExecFlags(factory);
#endif

#ifdef PHASE_FIELD_ENABLED
  PhaseFieldApp::registerExecFlags(factory);
#endif

#ifdef POROUS_FLOW_ENABLED
  PorousFlowApp::registerExecFlags(factory);
#endif

#ifdef RAY_TRACING_ENABLED
  RayTracingApp::registerExecFlags(factory);
#endif

#ifdef RDG_ENABLED
  RdgApp::registerExecFlags(factory);
#endif

#ifdef REACTOR_ENABLED
  ReactorApp::registerExecFlags(factory);
#endif

#ifdef RICHARDS_ENABLED
  RichardsApp::registerExecFlags(factory);
#endif

#ifdef SOLID_PROPERTIES_ENABLED
  SolidPropertiesApp::registerExecFlags(factory);
#endif

#ifdef STOCHASTIC_TOOLS_ENABLED
  StochasticToolsApp::registerExecFlags(factory);
#endif

#ifdef TENSOR_MECHANICS_ENABLED
  TensorMechanicsApp::registerExecFlags(factory);
#endif

#ifdef THERMAL_HYDRAULICS_ENABLED
  ThermalHydraulicsApp::registerExecFlags(factory);
#endif

#ifdef XFEM_ENABLED
  XFEMApp::registerExecFlags(factory);
#endif

  libmesh_ignore(factory);
}

void
ModulesApp::registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  mooseDeprecated(
      "\"registerAll\" in Modules is deprecated. Please update your *App.C file(s) to call the new "
      "templated \"registerAllObjects\" method (e.g. ModulesApp::registerAllObjects<MyApp>(...))");

#ifdef CHEMICAL_REACTIONS_ENABLED
  ChemicalReactionsApp::registerAll(f, af, s);
#endif

#ifdef CONTACT_ENABLED
  ContactApp::registerAll(f, af, s);
#endif

#ifdef ELECTROMAGNETICS_ENABLED
  ElectromagneticsApp::registerAll(f, af, s);
#endif

#ifdef FLUID_PROPERTIES_ENABLED
  FluidPropertiesApp::registerAll(f, af, s);
#endif

#ifdef FSI_ENABLED
  FsiApp::registerAll(f, af, s);
#endif

#ifdef GEOCHEMISTRY_ENABLED
  GeochemistryApp::registerAll(f, af, s);
#endif

#ifdef HEAT_CONDUCTION_ENABLED
  HeatConductionApp::registerAll(f, af, s);
#endif

#ifdef LEVEL_SET_ENABLED
  LevelSetApp::registerAll(f, af, s);
#endif

#ifdef MISC_ENABLED
  MiscApp::registerAll(f, af, s);
#endif

#ifdef NAVIER_STOKES_ENABLED
  NavierStokesApp::registerAll(f, af, s);
#endif

#ifdef OPTIMIZATION_ENABLED
  OptimizationApp::registerAll(f, af, s);
#endif

#ifdef PERIDYNAMICS_ENABLED
  PeridynamicsApp::registerAll(f, af, s);
#endif

#ifdef PHASE_FIELD_ENABLED
  PhaseFieldApp::registerAll(f, af, s);
#endif

#ifdef POROUS_FLOW_ENABLED
  PorousFlowApp::registerAll(f, af, s);
#endif

#ifdef RAY_TRACING_ENABLED
  RayTracingApp::registerAll(f, af, s);
#endif

#ifdef RDG_ENABLED
  RdgApp::registerAll(f, af, s);
#endif

#ifdef REACTOR_ENABLED
  ReactorApp::registerAll(f, af, s);
#endif

#ifdef RICHARDS_ENABLED
  RichardsApp::registerAll(f, af, s);
#endif

#ifdef SCALAR_TRANSPORT_ENABLED
  ScalarTransportApp::registerAll(f, af, s);
#endif

#ifdef SOLID_PROPERTIES_ENABLED
  SolidPropertiesApp::registerAll(f, af, s);
#endif

#ifdef STOCHASTIC_TOOLS_ENABLED
  StochasticToolsApp::registerAll(f, af, s);
#endif

#ifdef TENSOR_MECHANICS_ENABLED
  TensorMechanicsApp::registerAll(f, af, s);
#endif

#ifdef THERMAL_HYDRAULICS_ENABLED
  ThermalHydraulicsApp::registerAll(f, af, s);
#endif

#ifdef XFEM_ENABLED
  XFEMApp::registerAll(f, af, s);
#endif

#ifdef EXTERNAL_PETSC_SOLVER_ENABLED
  ExternalPetscSolverApp::registerAll(f, af, s);
#endif

  libmesh_ignore(f, s, af);
}

extern "C" void
ModulesApp__registerAll(Factory & f, ActionFactory & af, Syntax & s)
{
  ModulesApp::registerAll(f, af, s);
}

extern "C" void
ModulesApp__registerApps()
{
  ModulesApp::registerApps();
}

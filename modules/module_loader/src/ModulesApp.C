//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
#ifdef HEAT_TRANSFER_ENABLED
#include "HeatTransferApp.h"
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
#ifdef SOLID_MECHANICS_ENABLED
#include "SolidMechanicsApp.h"
#endif
#ifdef SOLID_PROPERTIES_ENABLED
#include "SolidPropertiesApp.h"
#endif
#ifdef STOCHASTIC_TOOLS_ENABLED
#include "StochasticToolsApp.h"
#endif
#ifdef SUBCHANNEL_ENABLED
#include "SubChannelApp.h"
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

#ifdef CHEMICAL_REACTIONS_ENABLED
  ChemicalReactionsApp::registerApps();
#endif

#ifdef CONTACT_ENABLED
  ContactApp::registerApps();
#endif

#ifdef ELECTROMAGNETICS_ENABLED
  ElectromagneticsApp::registerApps();
#endif

#ifdef FSI_ENABLED
  FsiApp::registerApps();
#endif

#ifdef FLUID_PROPERTIES_ENABLED
  FluidPropertiesApp::registerApps();
#endif

#ifdef FUNCTIONAL_EXPANSION_TOOLS_ENABLED
  FunctionalExpansionToolsApp::registerApps();
#endif

#ifdef GEOCHEMISTRY_ENABLED
  GeochemistryApp::registerApps();
#endif

#ifdef HEAT_TRANSFER_ENABLED
  HeatTransferApp::registerApps();
#endif

#ifdef LEVEL_SET_ENABLED
  LevelSetApp::registerApps();
#endif

#ifdef MISC_ENABLED
  MiscApp::registerApps();
#endif

#ifdef NAVIER_STOKES_ENABLED
  NavierStokesApp::registerApps();
#endif

#ifdef OPTIMIZATION_ENABLED
  OptimizationApp::registerApps();
#endif

#ifdef PERIDYNAMICS_ENABLED
  PeridynamicsApp::registerApps();
#endif

#ifdef PHASE_FIELD_ENABLED
  PhaseFieldApp::registerApps();
#endif

#ifdef POROUS_FLOW_ENABLED
  PorousFlowApp::registerApps();
#endif

#ifdef RAY_TRACING_ENABLED
  RayTracingApp::registerApps();
#endif

#ifdef RDG_ENABLED
  RdgApp::registerApps();
#endif

#ifdef REACTOR_ENABLED
  ReactorApp::registerApps();
#endif

#ifdef RICHARDS_ENABLED
  RichardsApp::registerApps();
#endif

#ifdef SCALAR_TRANSPORT_ENABLED
  ScalarTransportApp::registerApps();
#endif

#ifdef SOLID_MECHANICS_ENABLED
  SolidMechanicsApp::registerApps();
#endif

#ifdef SOLID_PROPERTIES_ENABLED
  SolidPropertiesApp::registerApps();
#endif

#ifdef STOCHASTIC_TOOLS_ENABLED
  StochasticToolsApp::registerApps();
#endif

#ifdef SUBCHANNEL_ENABLED
  SubChannelApp::registerApps();
#endif

#ifdef THERMAL_HYDRAULICS_ENABLED
  ThermalHydraulicsApp::registerApps();
#endif

#ifdef XFEM_ENABLED
  XFEMApp::registerApps();
#endif
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

#ifdef FUNCTIONAL_EXPANSION_TOOLS_ENABLED
  FunctionalExpansionToolsApp::registerAll(f, af, s);
#endif

#ifdef GEOCHEMISTRY_ENABLED
  GeochemistryApp::registerAll(f, af, s);
#endif

#ifdef HEAT_TRANSFER_ENABLED
  HeatTransferApp::registerAll(f, af, s);
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

#ifdef SOLID_MECHANICS_ENABLED
  SolidMechanicsApp::registerAll(f, af, s);
#endif

#ifdef SOLID_PROPERTIES_ENABLED
  SolidPropertiesApp::registerAll(f, af, s);
#endif

#ifdef STOCHASTIC_TOOLS_ENABLED
  StochasticToolsApp::registerAll(f, af, s);
#endif

#ifdef SUBCHANNEL_ENABLED
  SubChannelApp::registerAll(f, af, s);
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

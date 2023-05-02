//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"

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

class ModulesApp : public MooseApp
{
public:
  ModulesApp(const InputParameters & parameters);
  virtual ~ModulesApp();

  static InputParameters validParams();

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
  template <typename T>
  static void registerAllObjects(Factory & f, ActionFactory & af, Syntax & s);
  static void registerObjects(Factory & factory);
  static void associateSyntax(Syntax & syntax, ActionFactory & action_factory);
  static void registerExecFlags(Factory & factory);
};

template <typename T>
void
ModulesApp::registerAllObjects(Factory & f, ActionFactory & af, Syntax & s)
{
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

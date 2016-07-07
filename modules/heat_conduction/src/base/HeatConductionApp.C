/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "HeatConductionApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "MooseSyntax.h"

#include "AddSlaveFluxVectorAction.h"
#include "ConvectiveFluxFunction.h"
#include "GapConductance.h"
#include "GapConductanceConstraint.h"
#include "GapHeatPointSourceMaster.h"
#include "GapHeatTransfer.h"
#include "HeatConduction.h"
#include "AnisoHeatConduction.h"
#include "HeatConductionTimeDerivative.h"
#include "HeatConductionMaterial.h"
#include "AnisoHeatConductionMaterial.h"
#include "HeatConductionBC.h"
#include "HomogenizedHeatConduction.h"
#include "HomogenizedThermalConductivity.h"
#include "ThermalContactAuxBCsAction.h"
#include "ThermalContactAuxVarsAction.h"
#include "ThermalContactBCsAction.h"
#include "ThermalContactDiracKernelsAction.h"
#include "ThermalContactMaterialsAction.h"
#include "HeatSource.h"
#include "ThermalConductivity.h"
#include "CoupledConvectiveFlux.h"

template<>
InputParameters validParams<HeatConductionApp>()
{
  InputParameters params = validParams<MooseApp>();
  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  return params;
}

HeatConductionApp::HeatConductionApp(const InputParameters & parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  HeatConductionApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  HeatConductionApp::associateSyntax(_syntax, _action_factory);
}

HeatConductionApp::~HeatConductionApp()
{
}

// External entry point for dynamic application loading
extern "C" void HeatConductionApp__registerApps() { HeatConductionApp::registerApps(); }
void
HeatConductionApp::registerApps()
{
  registerApp(HeatConductionApp);
}

// External entry point for dynamic object registration
extern "C" void HeatConductionApp__registerObjects(Factory & factory) { HeatConductionApp::registerObjects(factory); }
void
HeatConductionApp::registerObjects(Factory & factory)
{
  registerNamedKernel(HeatConductionKernel, "HeatConduction");
  registerKernel(AnisoHeatConduction);
  registerKernel(HeatConductionTimeDerivative);
  registerKernel(HeatSource);
  registerKernel(HomogenizedHeatConduction);
  registerBoundaryCondition(HeatConductionBC);
  registerBoundaryCondition(ConvectiveFluxFunction);
  registerBoundaryCondition(GapHeatTransfer);
  registerBoundaryCondition(CoupledConvectiveFlux);
  registerMaterial(GapConductance);
  registerMaterial(HeatConductionMaterial);
  registerMaterial(AnisoHeatConductionMaterial);
  registerDiracKernel(GapHeatPointSourceMaster);
  registerPostprocessor(HomogenizedThermalConductivity);
  registerPostprocessor(ThermalConductivity);
  registerConstraint(GapConductanceConstraint);

  registerDeprecatedObjectName(ThermalConductivity, "ThermalCond", "09/15/2016 00:00");
  registerDeprecatedObjectName(HomogenizedHeatConduction, "HomogenizationHeatConduction", "09/15/2016 00:00");
}

// External entry point for dynamic syntax association
extern "C" void HeatConductionApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { HeatConductionApp::associateSyntax(syntax, action_factory); }
void
HeatConductionApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  // This registers an action to add the "slave_flux" vector to the system at the right time
  registerTask("add_slave_flux_vector", false);
  addTaskDependency("add_slave_flux_vector", "ready_to_init");
  addTaskDependency("init_problem", "add_slave_flux_vector");
  registerAction(AddSlaveFluxVectorAction, "add_slave_flux_vector");
  syntax.registerActionSyntax("AddSlaveFluxVectorAction", "ThermalContact/*");


  syntax.registerActionSyntax("ThermalContactAuxBCsAction",       "ThermalContact/*", "add_aux_kernel");
  syntax.registerActionSyntax("ThermalContactAuxVarsAction",      "ThermalContact/*", "add_aux_variable");
  syntax.registerActionSyntax("ThermalContactBCsAction",          "ThermalContact/*", "add_bc");
  syntax.registerActionSyntax("ThermalContactDiracKernelsAction", "ThermalContact/*", "add_dirac_kernel");
  syntax.registerActionSyntax("ThermalContactMaterialsAction",    "ThermalContact/*", "add_material");

  registerAction(ThermalContactAuxBCsAction,       "add_aux_kernel");
  registerAction(ThermalContactAuxVarsAction,      "add_aux_variable");
  registerAction(ThermalContactBCsAction,          "add_bc");
  registerAction(ThermalContactDiracKernelsAction, "add_dirac_kernel");
  registerAction(ThermalContactMaterialsAction,    "add_material");
}

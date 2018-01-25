//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
#include "HeatCapacityConductionTimeDerivative.h"
#include "ConsistentHeatCapacityTimeDerivative.h"
#include "SpecificHeatConductionTimeDerivative.h"
#include "ConsistentSpecificHeatTimeDerivative.h"
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
#include "ElectricalConductivity.h"
#include "SemiconductorLinearConductivity.h"
#include "JouleHeatingSource.h"

template <>
InputParameters
validParams<HeatConductionApp>()
{
  InputParameters params = validParams<MooseApp>();
  return params;
}

HeatConductionApp::HeatConductionApp(const InputParameters & parameters) : MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  HeatConductionApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  HeatConductionApp::associateSyntax(_syntax, _action_factory);

  Moose::registerExecFlags(_factory);
  HeatConductionApp::registerExecFlags(_factory);
}

HeatConductionApp::~HeatConductionApp() {}

// External entry point for dynamic application loading
extern "C" void
HeatConductionApp__registerApps()
{
  HeatConductionApp::registerApps();
}
void
HeatConductionApp::registerApps()
{
  registerApp(HeatConductionApp);
}

// External entry point for dynamic object registration
extern "C" void
HeatConductionApp__registerObjects(Factory & factory)
{
  HeatConductionApp::registerObjects(factory);
}
void
HeatConductionApp::registerObjects(Factory & factory)
{
  registerNamedKernel(HeatConductionKernel, "HeatConduction");

  registerKernel(AnisoHeatConduction);
  registerKernel(HeatConductionTimeDerivative);
  registerKernel(HeatSource);
  registerKernel(HomogenizedHeatConduction);
  registerKernel(HeatCapacityConductionTimeDerivative);
  registerKernel(ConsistentHeatCapacityTimeDerivative);
  registerKernel(JouleHeatingSource);
  registerKernel(SpecificHeatConductionTimeDerivative);
  registerKernel(ConsistentSpecificHeatTimeDerivative);

  registerBoundaryCondition(HeatConductionBC);
  registerBoundaryCondition(ConvectiveFluxFunction);
  registerBoundaryCondition(GapHeatTransfer);
  registerBoundaryCondition(CoupledConvectiveFlux);

  registerMaterial(ElectricalConductivity);
  registerMaterial(SemiconductorLinearConductivity);
  registerMaterial(GapConductance);
  registerMaterial(HeatConductionMaterial);
  registerMaterial(AnisoHeatConductionMaterial);

  registerDiracKernel(GapHeatPointSourceMaster);

  registerPostprocessor(HomogenizedThermalConductivity);
  registerPostprocessor(ThermalConductivity);

  registerConstraint(GapConductanceConstraint);
}

// External entry point for dynamic syntax association
extern "C" void
HeatConductionApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  HeatConductionApp::associateSyntax(syntax, action_factory);
}
void
HeatConductionApp::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  // This registers an action to add the "slave_flux" vector to the system at the right time
  registerTask("add_slave_flux_vector", false);
  addTaskDependency("add_slave_flux_vector", "ready_to_init");
  addTaskDependency("init_problem", "add_slave_flux_vector");
  registerAction(AddSlaveFluxVectorAction, "add_slave_flux_vector");
  registerSyntax("AddSlaveFluxVectorAction", "ThermalContact/*");

  registerSyntaxTask("ThermalContactAuxBCsAction", "ThermalContact/*", "add_aux_kernel");
  registerSyntaxTask("ThermalContactAuxVarsAction", "ThermalContact/*", "add_aux_variable");
  registerSyntaxTask("ThermalContactBCsAction", "ThermalContact/*", "add_bc");
  registerSyntaxTask("ThermalContactDiracKernelsAction", "ThermalContact/*", "add_dirac_kernel");
  registerSyntaxTask("ThermalContactMaterialsAction", "ThermalContact/*", "add_material");

  registerAction(ThermalContactAuxBCsAction, "add_aux_kernel");
  registerAction(ThermalContactAuxVarsAction, "add_aux_variable");
  registerAction(ThermalContactBCsAction, "add_bc");
  registerAction(ThermalContactDiracKernelsAction, "add_dirac_kernel");
  registerAction(ThermalContactMaterialsAction, "add_material");
}

// External entry point for dynamic execute flag registration
extern "C" void
HeatConductionApp__registerExecFlags(Factory & factory)
{
  HeatConductionApp::registerExecFlags(factory);
}
void
HeatConductionApp::registerExecFlags(Factory & /*factory*/)
{
}

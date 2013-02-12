#include "HeatConductionModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// heat_conduction
#include "AddSlaveFluxVectorAction.h"
#include "ConvectiveFluxFunction.h"
#include "GapConductance.h"
#include "GapHeatPointSourceMaster.h"
#include "GapHeatTransfer.h"
#include "HeatConduction.h"
#include "HeatConductionImplicitEuler.h"
#include "HeatConductionMaterial.h"
#include "BulkCoolantBC.h"
#include "ThermalContactAuxBCsAction.h"
#include "ThermalContactAuxKernelsAction.h"
#include "ThermalContactAuxVarsAction.h"
#include "ThermalContactBCsAction.h"
#include "ThermalContactDiracKernelsAction.h"
#include "ThermalContactMaterialsAction.h"
#include "HeatSource.h"
#include "HomogenizationHeatConduction.h"
#include "HomogenizedThermalConductivity.h"
#include "ThermalCond.h"

void
Elk::HeatConduction::registerObjects(Factory & factory)
{
  // heat_conduction
  registerNamedKernel(HeatConductionKernel, "HeatConduction");
  registerKernel(HeatConductionImplicitEuler);
  registerKernel(HeatSource);
  registerBoundaryCondition(ConvectiveFluxFunction);
  registerBoundaryCondition(GapHeatTransfer);
  registerBoundaryCondition(BulkCoolantBC);
  registerMaterial(GapConductance);
  registerMaterial(HeatConductionMaterial);
  registerDiracKernel(GapHeatPointSourceMaster);
  registerKernel(HomogenizationHeatConduction);
  registerPostprocessor(HomogenizedThermalConductivity);
  registerPostprocessor(ThermalCond);
}

void
Elk::HeatConduction::associateSyntax(Syntax & syntax, ActionFactory & action_factory)
{
  // This registers an action to add the "slave_flux" vector to the system at the right time
  registerActionName("add_slave_flux_vector", false);
  addActionNameDependency("add_slave_flux_vector", "ready_to_init");
  addActionNameDependency("init_problem", "add_slave_flux_vector");
  registerAction(AddSlaveFluxVectorAction, "add_slave_flux_vector");
  syntax.registerActionSyntax("AddSlaveFluxVectorAction", "ThermalContact/*");


  syntax.registerActionSyntax("ThermalContactAuxBCsAction",       "ThermalContact/*", "add_aux_kernel");
  syntax.registerActionSyntax("ThermalContactAuxKernelsAction",   "ThermalContact/*", "add_aux_kernel");
  syntax.registerActionSyntax("ThermalContactAuxVarsAction",      "ThermalContact/*", "add_aux_variable");
  syntax.registerActionSyntax("ThermalContactBCsAction",          "ThermalContact/*", "add_bc");
  syntax.registerActionSyntax("ThermalContactDiracKernelsAction", "ThermalContact/*", "add_dirac_kernel");
  syntax.registerActionSyntax("ThermalContactMaterialsAction",    "ThermalContact/*", "add_material");

  registerAction(ThermalContactAuxBCsAction,       "add_aux_kernel");
  registerAction(ThermalContactAuxKernelsAction,   "add_aux_kernel");
  registerAction(ThermalContactAuxVarsAction,      "add_aux_variablekernel");
  registerAction(ThermalContactBCsAction,          "add_bc");
  registerAction(ThermalContactDiracKernelsAction, "add_dirac_kernel");
  registerAction(ThermalContactMaterialsAction,    "add_material");
}

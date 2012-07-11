#include "HeatConductionModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// heat_conduction
#include "AddSlaveFluxVectorAction.h"
#include "GapConductance.h"
#include "GapHeatPointSourceMaster.h"
#include "GapHeatTransfer.h"
#include "HeatConduction.h"
#include "HeatConductionImplicitEuler.h"
#include "HeatConductionMaterial.h"
#include "BulkCoolantBC.h"
#include "ThermalContactAction.h"
#include "HeatSource.h"

void
Elk::HeatConduction::registerObjects()
{
  // heat_conduction
  registerNamedKernel(HeatConductionKernel, "HeatConduction");
  registerKernel(HeatConductionImplicitEuler);
  registerKernel(HeatSource);
  registerBoundaryCondition(GapHeatTransfer);
  registerBoundaryCondition(BulkCoolantBC);
  registerMaterial(GapConductance);
  registerMaterial(HeatConductionMaterial);
  registerDiracKernel(GapHeatPointSourceMaster);
}

void
Elk::HeatConduction::associateSyntax(Syntax & syntax)
{
  // This registers an action to add the "slave_flux" vector to the system at the right time
  registerActionName("add_slave_flux_vector", false);
  addActionNameDependency("add_slave_flux_vector", "ready_to_init");
  addActionNameDependency("init_problem", "add_slave_flux_vector");
  registerAction(AddSlaveFluxVectorAction, "add_slave_flux_vector");

  // thermal contact
  syntax.registerActionSyntax("ThermalContactAction", "ThermalContact/*");
  registerAction(ThermalContactAction, "meta_action");
}

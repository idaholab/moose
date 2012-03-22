#include "HeatConductionModule.h"
#include "Factory.h"
#include "ActionFactory.h"

// heat_conduction
#include "AddSlaveFluxVectorAction.h"
#include "GapHeatPointSourceMaster.h"
#include "GapHeatTransfer.h"
#include "HeatConduction.h"
#include "HeatConductionImplicitEuler.h"
#include "HeatConductionMaterial.h"
#include "BulkCoolantBC.h"
#include "ThermalContactAction.h"

void
Elk::HeatConduction::registerObjects()
{
  // heat_conduction
  registerNamedKernel(HeatConductionKernel, "HeatConduction");
  registerKernel(HeatConductionImplicitEuler);
  registerBoundaryCondition(GapHeatTransfer);
  registerBoundaryCondition(BulkCoolantBC);
  registerMaterial(HeatConductionMaterial);
  registerDiracKernel(GapHeatPointSourceMaster);
   
  // This registers an action to add the "slave_flux" vector to the system at the right time
  registerActionName("add_slave_flux_vector", false);
  addActionNameDependency("add_slave_flux_vector", "ready_to_init");
  addActionNameDependency("init_problem", "add_slave_flux_vector");
  registerAction(AddSlaveFluxVectorAction, "add_slave_flux_vector");

  // thermal contact
  registerAction(ThermalContactAction, "meta_action");
}

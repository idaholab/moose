#include "HeatConductionModule.h"
#include "Factory.h"
#include "ActionFactory.h"
#include "Parser.h"

// heat_conduction
#include "AddSlaveFluxVectorAction.h"
#include "GapHeatPointSourceMaster.h"
#include "GapHeatTransfer.h"
#include "HeatConduction.h"
#include "HeatConductionImplicitEuler.h"
#include "HeatConductionMaterial.h"
#include "SiHeatConductionMaterial.h"
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
  registerMaterial(SiHeatConductionMaterial);
  registerDiracKernel(GapHeatPointSourceMaster);
}

void
Elk::HeatConduction::associateSyntax()
{
  // This registers an action to add the "slave_flux" vector to the system at the right time
  registerActionName("add_slave_flux_vector", false);
  addActionNameDependency("add_slave_flux_vector", "ready_to_init");
  addActionNameDependency("init_problem", "add_slave_flux_vector");
  registerAction(AddSlaveFluxVectorAction, "add_slave_flux_vector");

  // thermal contact
  Moose::syntax.registerActionSyntax("ThermalContactAction", "ThermalContact/*");
  registerAction(ThermalContactAction, "meta_action");
}

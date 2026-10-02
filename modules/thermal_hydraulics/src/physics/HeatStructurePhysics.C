//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructurePhysics.h"

registerPhysicsBaseTasks("ThermalHydraulicsApp", HeatStructurePhysics);
registerMooseAction("ThermalHydraulicsApp", HeatStructurePhysics, "add_kernel");
registerMooseAction("ThermalHydraulicsApp", HeatStructurePhysics, "add_bc");
registerMooseAction("ThermalHydraulicsApp", HeatStructurePhysics, "add_variables_physics");
registerMooseAction("ThermalHydraulicsApp", HeatStructurePhysics, "add_ics_physics");
registerMooseAction("ThermalHydraulicsApp", HeatStructurePhysics, "add_preconditioning");

InputParameters
HeatStructurePhysics::validParams()
{
  return HeatConductionCG::validParams();
}

HeatStructurePhysics::HeatStructurePhysics(const InputParameters & params)
  : HeatConductionCG(params),
    _coordinator(THMVariableCoordinator::findOrCreate(_awh, _action_factory))
{
}

void
HeatStructurePhysics::addSolverVariables()
{
  _coordinator.requestVariable(true, _temperature_name, "LAGRANGE", "FIRST", 1.0, _blocks);
  saveSolverVariableName(_temperature_name);
}

void
HeatStructurePhysics::addInitialConditions()
{
  if (getParam<bool>("initialize_variables_from_mesh_file"))
    return;

  const std::string class_name = "FunctionIC";
  InputParameters params = getFactory().getValidParams(class_name);
  params.set<VariableName>("variable") = _temperature_name;
  assignBlocks(params, _blocks);
  params.set<FunctionName>("function") = getParam<FunctionName>("initial_temperature");
  getProblem().addInitialCondition(class_name, prefix() + _temperature_name + "_ic", params);
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatStructurePlateAC.h"

// Registered under the classic Component's name ('HeatStructurePlate') rather than this class's
// own name; see FlowChannel1PhaseAC.C for why.
registerMooseActionAliased("ThermalHydraulicsApp",
                           HeatStructurePlateAC,
                           "HeatStructurePlate",
                           "add_mesh_generator");
registerMooseActionAliased("ThermalHydraulicsApp",
                           HeatStructurePlateAC,
                           "HeatStructurePlate",
                           "init_component_physics");
registerMooseActionAliased("ThermalHydraulicsApp",
                           HeatStructurePlateAC,
                           "HeatStructurePlate",
                           "add_material");
registerActionComponentAliased("ThermalHydraulicsApp", HeatStructurePlateAC, "HeatStructurePlate");

InputParameters
HeatStructurePlateAC::validParams()
{
  InputParameters params = HeatStructureBaseAC::validParams();
  params.addRequiredParam<Real>("depth", "Dimension of the plate in the third direction [m]");
  params.addClassDescription("Plate heat structure.");
  return params;
}

HeatStructurePlateAC::HeatStructurePlateAC(const InputParameters & params)
  : ActionComponent(params), HeatStructureBaseAC(params), _depth(getParam<Real>("depth"))
{
}

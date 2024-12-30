//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsHeatTransferFromHeatFlux.h"
#include "FlowModelSinglePhase.h"

registerMooseObjectAliased("ThermalHydraulicsApp",
                           PhysicsHeatTransferFromHeatFlux,
                           "HeatTransferFromHeatFlux");

InputParameters
PhysicsHeatTransferFromHeatFlux::validParams()
{
  InputParameters params = PhysicsHeatTransferBase::validParams();

  params.addRequiredParam<MooseFunctorName>("q_wall", "Specified wall heat flux [W/m^2]");
  params.addClassDescription(
      "Heat transfer specified by a wall heat flux into a channel using Physics.");
  return params;
}

PhysicsHeatTransferFromHeatFlux::PhysicsHeatTransferFromHeatFlux(const InputParameters & parameters)
  : PhysicsHeatTransferBase(parameters)
{
}

void
PhysicsHeatTransferFromHeatFlux::init()
{
  PhysicsHeatTransferBase::init();
  for (auto th_phys : _th_physics)
    th_phys->addWallHeatFlux(name(), ThermalHydraulicsFlowPhysics::FixedHeatFlux);
}

void
PhysicsHeatTransferFromHeatFlux::addMooseObjects()
{
  PhysicsHeatTransferBase::addMooseObjects();
}

bool
PhysicsHeatTransferFromHeatFlux::isTemperatureType() const
{
  return false;
}

const MooseFunctorName
PhysicsHeatTransferFromHeatFlux::getWallHeatFluxFunctorName() const
{
  return getParam<MooseFunctorName>("q_wall");
}

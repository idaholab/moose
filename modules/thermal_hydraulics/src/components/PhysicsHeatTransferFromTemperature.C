//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsHeatTransferFromTemperature.h"
#include "FlowModelSinglePhase.h"

registerMooseObject("ThermalHydraulicsApp", PhysicsHeatTransferFromTemperature);

InputParameters
PhysicsHeatTransferFromTemperature::validParams()
{
  InputParameters params = PhysicsHeatTransferBase::validParams();

  MooseEnum var_type("nodal elemental", "nodal", false);
  params.addParam<MooseEnum>(
      "var_type", var_type, "The type of wall temperature variable (nodal, elemental).");
  params.addClassDescription(
      "Heat transfer specified by a wall temperature into a channel using Physics.");
  return params;
}

PhysicsHeatTransferFromTemperature::PhysicsHeatTransferFromTemperature(
    const InputParameters & parameters)
  : PhysicsHeatTransferBase(parameters),
    _fe_type(getParam<MooseEnum>("var_type") == 0 ? FEType(FIRST, LAGRANGE)
                                                  : FEType(CONSTANT, MONOMIAL))
{
}

void
PhysicsHeatTransferFromTemperature::init()
{
  PhysicsHeatTransferBase::init();
  for (auto th_phys : _th_physics)
    th_phys->addWallHeatFlux(name(), ThermalHydraulicsFlowPhysics::FixedWallTemperature);
}

const FEType &
PhysicsHeatTransferFromTemperature::getFEType()
{
  return _fe_type;
}

void
PhysicsHeatTransferFromTemperature::addVariables()
{
  PhysicsHeatTransferBase::addVariables();

  // Add the functor
  if (!getTHMProblem().hasFunctor(_T_wall_name, 0))
  {
    // This auxiliary variable will receive the temperature
    getTHMProblem().addSimVariable(false, _T_wall_name, getFEType(), _flow_channel_subdomains);
  }
}

void
PhysicsHeatTransferFromTemperature::addMooseObjects()
{
  PhysicsHeatTransferBase::addMooseObjects();
}

bool
PhysicsHeatTransferFromTemperature::isTemperatureType() const
{
  return true;
}

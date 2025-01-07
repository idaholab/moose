//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsInletStagnationPressureTemperature.h"

registerMooseObjectAliased("ThermalHydraulicsApp",
                           PhysicsInletStagnationPressureTemperature,
                           "InletStagnationPressureTemperature");

InputParameters
PhysicsInletStagnationPressureTemperature::validParams()
{
  InputParameters params = PhysicsFlowBoundary::validParams();
  params.addRequiredParam<Real>("p0", "Prescribed stagnation pressure [Pa]");
  params.addRequiredParam<Real>("T0", "Prescribed stagnation temperature [K]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");
  params.declareControllable("p0 T0");
  params.addClassDescription("Boundary condition with prescribed stagnation pressure and "
                             "temperature for flow channels, using the Physics implementation.");
  return params;
}

PhysicsInletStagnationPressureTemperature::PhysicsInletStagnationPressureTemperature(
    const InputParameters & params)
  : PhysicsFlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
PhysicsInletStagnationPressureTemperature::init()
{
  PhysicsFlowBoundary::init();
  // For now, we do this here. We could consider doing it elsewhere
  for (auto th_phys : _th_physics)
    th_phys->setInlet(name(), ThermalHydraulicsFlowPhysics::StagnationPressureTemperature);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsInletVelocityTemperature.h"
#include "ThermalHydraulicsFlowPhysics.h"

registerMooseObjectAliased("ThermalHydraulicsApp",
                           PhysicsInletVelocityTemperature,
                           "InletVelocityTemperature");

InputParameters
PhysicsInletVelocityTemperature::validParams()
{
  InputParameters params = PhysicsFlowBoundary::validParams();
  params.addRequiredParam<Real>("vel", "Prescribed velocity [m/s]");
  params.addRequiredParam<Real>("T", "Prescribed temperature [K]");
  params.addParam<bool>("reversible", true, "True for reversible, false for pure inlet");
  params.declareControllable("vel T");
  params.addClassDescription("Boundary condition with prescribed velocity and temperature "
                             "for flow channels using thermal hydraulics Physics.");
  return params;
}

PhysicsInletVelocityTemperature::PhysicsInletVelocityTemperature(const InputParameters & params)
  : PhysicsFlowBoundary(params), _reversible(getParam<bool>("reversible"))
{
}

void
PhysicsInletVelocityTemperature::check() const
{
  PhysicsFlowBoundary::check();
}

void
PhysicsInletVelocityTemperature::init()
{
  PhysicsFlowBoundary::init();
  // For now, we do this here. We could consider doing it elsewhere
  for (auto th_phys : _th_physics)
    th_phys->setInlet(name(), ThermalHydraulicsFlowPhysics::VelocityTemperature);
}

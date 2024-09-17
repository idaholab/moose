//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsHeatTransferBase.h"
#include "FlowModelSinglePhase.h"
#include "PhysicsFlowChannel.h"
#include "ClosuresBase.h"
#include "MooseUtils.h"
#include "ThermalHydraulicsFlowPhysics.h"

InputParameters
PhysicsHeatTransferBase::validParams()
{
  InputParameters params = HeatTransferBase::validParams();
  params.addParam<FunctionName>("Hw", "Convective heat transfer coefficient [W/(m^2-K)]");
  params.declareControllable("Hw");
  return params;
}

PhysicsHeatTransferBase::PhysicsHeatTransferBase(const InputParameters & parameters)
  : HeatTransferBase(parameters)
{
}

void
PhysicsHeatTransferBase::init()
{
  HeatTransferBase::init();

  if (hasComponentByName<PhysicsFlowChannel>(_flow_channel_name))
  {
    const PhysicsFlowChannel & flow_channel =
        getComponentByName<PhysicsFlowChannel>(_flow_channel_name);
    for (const auto & physics : flow_channel.getPhysics())
      _th_physics.insert(physics);
    // NOTE: we currently expect to error on non thermal-hydraulics physics.
    // This may be removed in the future
  }

  // There's two options here:
  // - from a specified temperature (htc coming from closure)
  // - from a specified heat flux
}

void
PhysicsHeatTransferBase::initSecondary()
{
  HeatTransferBase::initSecondary();

  // determine names of heat transfer variables
  if (hasComponentByName<PhysicsFlowChannel>(_flow_channel_name))
  {
    const PhysicsFlowChannel & flow_channel =
        getComponentByName<PhysicsFlowChannel>(_flow_channel_name);
    const std::string Hw_suffix = flow_channel.getHeatTransferNamesSuffix(name());

    _Hw_name = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL + Hw_suffix;
  }
  else
    logError("Coupled component '", _flow_channel_name, "' must be a Physics flow channel.");
}

void
PhysicsHeatTransferBase::check() const
{
  HeatTransferBase::check();

  if (_closures != nullptr && hasComponentByName<PhysicsFlowChannel>(_flow_channel_name))
    _closures->checkHeatTransfer(*this, getComponentByName<PhysicsFlowChannel>(_flow_channel_name));
}

void
PhysicsHeatTransferBase::addMooseObjects()
{
  HeatTransferBase::addMooseObjects();

  _closures->addMooseObjectsHeatTransfer(
      *this, getComponentByName<PhysicsFlowChannel>(_flow_channel_name));
}

const std::string &
PhysicsHeatTransferBase::getWallHeatTransferCoefficientName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _Hw_name;
}

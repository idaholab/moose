//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatTransfer1PhaseBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowChannel1Phase.h"
#include "ClosuresBase.h"
#include "MooseUtils.h"

InputParameters
HeatTransfer1PhaseBase::validParams()
{
  InputParameters params = HeatTransferBase::validParams();
  params.addParam<FunctionName>("Hw", "Convective heat transfer coefficient [W/(m^2-K)]");
  params.declareControllable("Hw");
  return params;
}

HeatTransfer1PhaseBase::HeatTransfer1PhaseBase(const InputParameters & parameters)
  : HeatTransferBase(parameters)
{
}

void
HeatTransfer1PhaseBase::init()
{
  HeatTransferBase::init();
}

void
HeatTransfer1PhaseBase::initSecondary()
{
  HeatTransferBase::initSecondary();

  // determine names of heat transfer variables
  if (hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const FlowChannel1Phase & flow_channel =
        getComponentByName<FlowChannel1Phase>(_flow_channel_name);
    const std::string Hw_suffix = flow_channel.getHeatTransferNamesSuffix(name());

    _Hw_1phase_name = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL + Hw_suffix;
  }
  else
    logError("Coupled component '", _flow_channel_name, "' must be a single phase flow channel.");
}

void
HeatTransfer1PhaseBase::check() const
{
  HeatTransferBase::check();

  for (const auto & closures : _closures_objects)
    if (closures && hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
      closures->checkHeatTransfer(*this, getComponentByName<FlowChannel1Phase>(_flow_channel_name));
}

void
HeatTransfer1PhaseBase::addMooseObjects()
{
  HeatTransferBase::addMooseObjects();

  for (const auto & closures : _closures_objects)
    closures->addMooseObjectsHeatTransfer(
        *this, getComponentByName<FlowChannel1Phase>(_flow_channel_name));
}

const MaterialPropertyName &
HeatTransfer1PhaseBase::getWallHeatTransferCoefficient1PhaseName() const
{
  checkSetupStatus(INITIALIZED_SECONDARY);

  return _Hw_1phase_name;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WallTemperature1PhaseClosures.h"
#include "FlowChannel1Phase.h"
#include "HeatTransfer1PhaseBase.h"

registerMooseObject("ThermalHydraulicsApp", WallTemperature1PhaseClosures);

InputParameters
WallTemperature1PhaseClosures::validParams()
{
  InputParameters params = Closures1PhaseBase::validParams();
  params.addClassDescription("Adds wall temperature material for single-phase flow.");
  return params;
}

WallTemperature1PhaseClosures::WallTemperature1PhaseClosures(const InputParameters & params)
  : Closures1PhaseBase(params)
{
}

void
WallTemperature1PhaseClosures::checkFlowChannel(const FlowChannelBase & /*flow_channel*/) const
{
}

void
WallTemperature1PhaseClosures::checkHeatTransfer(const HeatTransferBase & /*heat_transfer*/,
                                                 const FlowChannelBase & /*flow_channel*/) const
{
}

void
WallTemperature1PhaseClosures::addMooseObjectsFlowChannel(const FlowChannelBase & flow_channel)
{
  const FlowChannel1Phase & flow_channel_1phase =
      dynamic_cast<const FlowChannel1Phase &>(flow_channel);

  const unsigned int n_ht_connections = flow_channel_1phase.getNumberOfHeatTransferConnections();
  if ((n_ht_connections > 0) && (flow_channel.getTemperatureMode()))
  {
    if (flow_channel.getNumberOfHeatTransferConnections() > 1)
      addAverageWallTemperatureMaterial(flow_channel_1phase);
    else
      addWallTemperatureFromAuxMaterial(flow_channel_1phase);
  }
}

void
WallTemperature1PhaseClosures::addMooseObjectsHeatTransfer(
    const HeatTransferBase & /*heat_transfer*/, const FlowChannelBase & /*flow_channel*/)
{
}

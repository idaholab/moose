//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WallTemperature1PhaseClosures.h"
#include "FlowChannelClosuresInterface.h"

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
WallTemperature1PhaseClosures::checkFlowChannel(
    const FlowChannelClosuresInterface & /*flow_channel*/) const
{
}

void
WallTemperature1PhaseClosures::checkHeatTransfer(
    const HeatTransferClosuresInterface & /*heat_transfer*/,
    const FlowChannelClosuresInterface & /*flow_channel*/) const
{
}

void
WallTemperature1PhaseClosures::addMooseObjectsFlowChannel(
    const FlowChannelClosuresInterface & flow_channel)
{
  const unsigned int n_ht_connections = flow_channel.getClosuresNumberOfHeatTransferConnections();
  if ((n_ht_connections > 0) && (flow_channel.getClosuresTemperatureMode()))
  {
    for (unsigned int i = 0; i < n_ht_connections; i++)
      addWallTemperatureFromAuxMaterial(flow_channel, i);

    if (n_ht_connections > 1)
      addAverageWallTemperatureMaterial(flow_channel);
  }
}

void
WallTemperature1PhaseClosures::addMooseObjectsHeatTransfer(
    const HeatTransferClosuresInterface & /*heat_transfer*/,
    const FlowChannelClosuresInterface & /*flow_channel*/)
{
}

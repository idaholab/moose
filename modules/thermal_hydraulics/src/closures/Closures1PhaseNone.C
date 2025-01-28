//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Closures1PhaseNone.h"
#include "FlowChannel1Phase.h"
#include "HeatTransfer1PhaseBase.h"

registerMooseObject("ThermalHydraulicsApp", Closures1PhaseNone);

InputParameters
Closures1PhaseNone::validParams()
{
  InputParameters params = Closures1PhaseBase::validParams();
  params.addParam<bool>("add_wall_temperature_property",
                        true,
                        "If true, create a material property for the wall temperature (average if "
                        "multiple heat transfers)");
  params.addClassDescription("No 1-phase closures. Useful for testing with one-time correlations.");
  return params;
}

Closures1PhaseNone::Closures1PhaseNone(const InputParameters & params) : Closures1PhaseBase(params)
{
  if (getParam<bool>("add_wall_temperature_property"))
    mooseDeprecated("'Closures1PhaseNone' is deprecated. Since 'add_wall_temperature_property' was "
                    "set to 'true', change the type of the 'Closures1PhaseNone' object to "
                    "'WallTemperature1PhaseClosures'.");
  else
    mooseDeprecated(
        "'Closures1PhaseNone' is deprecated. The 'closures' parameter now takes an arbitrary-sized "
        "list of closures objects, defaulting to an empty list, so to transition, delete the "
        "'Closures1PhaseNone' block and the 'closures' parameter from your input file.");
}

void
Closures1PhaseNone::checkFlowChannel(const FlowChannelBase & /*flow_channel*/) const
{
}

void
Closures1PhaseNone::checkHeatTransfer(const HeatTransferBase & /*heat_transfer*/,
                                      const FlowChannelBase & /*flow_channel*/) const
{
}

void
Closures1PhaseNone::addMooseObjectsFlowChannel(const FlowChannelBase & flow_channel)
{
  if (getParam<bool>("add_wall_temperature_property"))
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
}

void
Closures1PhaseNone::addMooseObjectsHeatTransfer(const HeatTransferBase & /*heat_transfer*/,
                                                const FlowChannelBase & /*flow_channel*/)
{
}

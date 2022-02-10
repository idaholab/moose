//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Closures1PhaseSimple.h"
#include "FlowModelSinglePhase.h"
#include "FlowChannel1Phase.h"
#include "HeatTransfer1PhaseBase.h"

registerMooseObject("ThermalHydraulicsApp", Closures1PhaseSimple);

InputParameters
Closures1PhaseSimple::validParams()
{
  InputParameters params = Closures1PhaseBase::validParams();

  params.addClassDescription("Simple 1-phase closures");

  return params;
}

Closures1PhaseSimple::Closures1PhaseSimple(const InputParameters & params)
  : Closures1PhaseBase(params)
{
}

void
Closures1PhaseSimple::checkFlowChannel(const FlowChannelBase & flow_channel) const
{
  if (!flow_channel.isParamValid("f"))
    logComponentError(flow_channel.cname(),
                      "When using simple closures, the parameter 'f' must be provided.");
}

void
Closures1PhaseSimple::checkHeatTransfer(const HeatTransferBase & heat_transfer,
                                        const FlowChannelBase & /*flow_channel*/) const
{
  if (!heat_transfer.isParamValid("Hw"))
    logComponentError(heat_transfer.cname(),
                      "The parameter 'Hw' must be provided when using simple closures.");
}

void
Closures1PhaseSimple::addMooseObjectsFlowChannel(const FlowChannelBase & flow_channel)
{
  const FlowChannel1Phase & flow_channel_1phase =
      dynamic_cast<const FlowChannel1Phase &>(flow_channel);

  // wall friction material
  addWallFrictionFunctionMaterial(flow_channel_1phase);

  const unsigned int n_ht_connections = flow_channel_1phase.getNumberOfHeatTransferConnections();
  if (n_ht_connections > 0)
  {
    // wall heat transfer coefficient material
    if (n_ht_connections > 1)
      addWeightedAverageMaterial(flow_channel_1phase,
                                 flow_channel_1phase.getWallHTCNames1Phase(),
                                 flow_channel_1phase.getHeatedPerimeterNames(),
                                 FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL);

    // wall temperature material
    if (flow_channel_1phase.getTemperatureMode())
    {
      if (n_ht_connections > 1)
        addAverageWallTemperatureMaterial(flow_channel_1phase);
      else
        addWallTemperatureFromAuxMaterial(flow_channel_1phase);
    }
    else
    {
      if (n_ht_connections > 1)
        addWallTemperatureFromHeatFluxMaterial(flow_channel_1phase);
    }
  }
}

void
Closures1PhaseSimple::addMooseObjectsHeatTransfer(const HeatTransferBase & heat_transfer,
                                                  const FlowChannelBase & flow_channel)
{
  const HeatTransfer1PhaseBase & heat_transfer_1phase =
      dynamic_cast<const HeatTransfer1PhaseBase &>(heat_transfer);
  const FunctionName & Hw_fn_name = heat_transfer.getParam<FunctionName>("Hw");

  {
    const std::string class_name = "ADGenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<std::vector<std::string>>("prop_names") = {
        heat_transfer_1phase.getWallHeatTransferCoefficient1PhaseName()};
    params.set<std::vector<FunctionName>>("prop_values") = {Hw_fn_name};
    _sim.addMaterial(
        class_name, genName(heat_transfer.name(), "Hw_material", flow_channel.name()), params);
  }

  heat_transfer.makeFunctionControllableIfConstant(Hw_fn_name, "Hw");
}

void
Closures1PhaseSimple::addWallTemperatureFromHeatFluxMaterial(
    const FlowChannel1Phase & flow_channel) const
{
  const std::string class_name = "ADTemperatureWall3EqnMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
  params.set<MaterialPropertyName>("q_wall") = FlowModel::HEAT_FLUX_WALL;
  params.set<MaterialPropertyName>("Hw") = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL;
  _sim.addMaterial(class_name, genName(flow_channel.name(), "T_wall_mat"), params);
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Closures1PhaseSimple.h"
#include "FlowChannelClosuresInterface.h"
#include "HeatTransferClosuresInterface.h"
#include "FlowModelSinglePhase.h"
#include "FEProblemBase.h"

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
Closures1PhaseSimple::checkFlowChannel(const FlowChannelClosuresInterface & flow_channel) const
{
  if (!flow_channel.hasClosuresWallFrictionFactorFunction())
    logComponentError(flow_channel.getClosuresName(),
                      "When using simple closures, the parameter 'f' must be provided.");
}

void
Closures1PhaseSimple::checkHeatTransfer(const HeatTransferClosuresInterface & heat_transfer,
                                        const FlowChannelClosuresInterface & /*flow_channel*/) const
{
  if (!heat_transfer.hasClosuresWallHeatTransferCoefficientFunction())
    logComponentError(heat_transfer.getClosuresName(),
                      "The parameter 'Hw' must be provided when using simple closures.");
}

void
Closures1PhaseSimple::addMooseObjectsFlowChannel(const FlowChannelClosuresInterface & flow_channel)
{
  // wall friction material
  addWallFrictionFunctionMaterial(flow_channel);

  const unsigned int n_ht_connections = flow_channel.getClosuresNumberOfHeatTransferConnections();
  if (n_ht_connections > 0)
  {
    // wall heat transfer coefficient material
    if (n_ht_connections > 1)
      addWeightedAverageMaterial(flow_channel,
                                 flow_channel.getClosuresWallHTCNames(),
                                 flow_channel.getClosuresHeatedPerimeterNames(),
                                 FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL);

    // wall temperature material
    if (flow_channel.getClosuresTemperatureMode())
    {
      if (n_ht_connections > 1)
        addAverageWallTemperatureMaterial(flow_channel);
      else
        addWallTemperatureFromAuxMaterial(flow_channel);
    }
    else
    {
      if (n_ht_connections > 1)
        addWallTemperatureFromHeatFluxMaterial(flow_channel);
    }
  }
}

void
Closures1PhaseSimple::addMooseObjectsHeatTransfer(
    const HeatTransferClosuresInterface & heat_transfer,
    const FlowChannelClosuresInterface & flow_channel)
{
  const FunctionName & Hw_fn_name = heat_transfer.getClosuresWallHeatTransferCoefficientFunction();

  {
    const std::string class_name = "ADGenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getClosuresBlocks();
    params.set<std::vector<std::string>>("prop_names") = {
        heat_transfer.getClosuresWallHeatTransferCoefficient1PhaseName()};
    params.set<std::vector<FunctionName>>("prop_values") = {Hw_fn_name};
    _problem.addMaterial(
        class_name,
        genName(heat_transfer.getClosuresName(), "Hw_material", flow_channel.getClosuresName()),
        params);
  }
}

void
Closures1PhaseSimple::addWallTemperatureFromHeatFluxMaterial(
    const FlowChannelClosuresInterface & flow_channel) const
{
  const std::string class_name = "ADTemperatureWall3EqnMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getClosuresBlocks();
  params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
  params.set<MaterialPropertyName>("q_wall") = FlowModel::HEAT_FLUX_WALL;
  params.set<MaterialPropertyName>("Hw") = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL;
  _problem.addMaterial(class_name, genName(flow_channel.getClosuresName(), "T_wall_mat"), params);
}

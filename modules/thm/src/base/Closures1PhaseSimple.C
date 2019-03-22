#include "Closures1PhaseSimple.h"
#include "FlowModelSinglePhase.h"
#include "FlowChannel1Phase.h"

registerMooseObject("THMApp", Closures1PhaseSimple);

template <>
InputParameters
validParams<Closures1PhaseSimple>()
{
  InputParameters params = validParams<Closures1PhaseBase>();

  params.addClassDescription("Simple 1-phase closures");

  return params;
}

Closures1PhaseSimple::Closures1PhaseSimple(const InputParameters & params)
  : Closures1PhaseBase(params)
{
}

void
Closures1PhaseSimple::check(const FlowChannelBase & flow_channel) const
{
  if (!flow_channel.isParamValid("f"))
    logError("When using simple closures, the parameter 'f' must be provided.");
}

void
Closures1PhaseSimple::addMooseObjects(const FlowChannelBase & flow_channel)
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
      addWallTemperatureFromHeatFluxMaterial(flow_channel_1phase);
  }
}

void
Closures1PhaseSimple::addWallTemperatureFromHeatFluxMaterial(
    const FlowChannel1Phase & flow_channel) const
{
  const std::string class_name = "TemperatureWall3EqnMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
  params.set<MaterialPropertyName>("q_wall") = FlowModel::HEAT_FLUX_WALL;
  params.set<MaterialPropertyName>("Hw") = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL;
  _sim.addMaterial(class_name, Component::genName(flow_channel.name(), class_name), params);
}

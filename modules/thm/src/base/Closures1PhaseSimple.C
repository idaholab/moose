#include "Closures1PhaseSimple.h"
#include "FlowModelSinglePhase.h"
#include "Pipe.h"

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
Closures1PhaseSimple::check(const Pipe & flow_channel) const
{
  if (!flow_channel.isParamValid("f"))
    logError("When using simple closures, the parameter 'f' must be provided.");
}

void
Closures1PhaseSimple::addMooseObjects(const Pipe & flow_channel)
{
  // wall friction material
  addWallFrictionFunctionMaterial(flow_channel);

  // wall heat transfer coefficient material
  const unsigned int n_ht_connections = flow_channel.getNumberOfHeatTransferConnections();
  if (n_ht_connections > 1)
    addWeightedAverageMaterial(flow_channel,
                               flow_channel.getWallHTCNames1Phase(),
                               flow_channel.getHeatedPerimeterNames(),
                               FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL);
  else if (n_ht_connections == 0)
    addZeroMaterial(flow_channel, FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL);

  // wall temperature material
  if (flow_channel.getTemperatureMode())
  {
    if (flow_channel.getNumberOfHeatTransferConnections() > 1)
      addAverageWallTemperatureMaterial(flow_channel);
    else
      addWallTemperatureFromAuxMaterial(flow_channel);
  }
  else
    addWallTemperatureFromHeatFluxMaterial(flow_channel);
}

void
Closures1PhaseSimple::addWallTemperatureFromHeatFluxMaterial(const Pipe & flow_channel) const
{
  const std::string class_name = "TemperatureWall3EqnMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<MaterialPropertyName>("T") = FlowModelSinglePhase::TEMPERATURE;
  params.set<std::vector<VariableName>>("q_wall") = {FlowModel::HEAT_FLUX_WALL};
  params.set<MaterialPropertyName>("Hw") = FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL;
  _sim.addMaterial(class_name, flow_channel.genName(flow_channel.name(), class_name), params);
}

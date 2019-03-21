#include "ClosuresBase.h"
#include "FlowChannelBase.h"

template <>
InputParameters
validParams<ClosuresBase>()
{
  InputParameters params = validParams<MooseObject>();

  params.addPrivateParam<Simulation *>("_sim");

  params.registerBase("THM:closures");

  return params;
}

ClosuresBase::ClosuresBase(const InputParameters & params)
  : MooseObject(params),
    LoggingInterface(dynamic_cast<THMApp &>(getMooseApp()), name()),

    _sim(*params.getCheckedPointerParam<Simulation *>("_sim")),
    _factory(_app.getFactory())
{
}

void
ClosuresBase::addZeroMaterial(const FlowChannelBase & flow_channel,
                              const std::string & property_name) const
{
  const std::string class_name = "ConstantMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<std::string>("property_name") = property_name;
  params.set<Real>("value") = 0;
  _sim.addMaterial(
      class_name, Component::genName(flow_channel.name(), class_name, property_name), params);
}

void
ClosuresBase::addWeightedAverageMaterial(const FlowChannelBase & flow_channel,
                                         const std::vector<MaterialPropertyName> & values,
                                         const std::vector<VariableName> & weights,
                                         const MaterialPropertyName & property_name) const
{
  const std::string class_name = "WeightedAverageMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<MaterialPropertyName>("prop_name") = property_name;
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<std::vector<MaterialPropertyName>>("values") = values;
  params.set<std::vector<VariableName>>("weights") = weights;
  _sim.addMaterial(
      class_name, Component::genName(flow_channel.name(), class_name, property_name), params);
}

void
ClosuresBase::addWallTemperatureFromAuxMaterial(const FlowChannelBase & flow_channel) const
{
  const std::string class_name = "CoupledVariableValueMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<MaterialPropertyName>("prop_name") = {FlowModel::TEMPERATURE_WALL};
  params.set<std::vector<VariableName>>("coupled_variable") = {FlowModel::TEMPERATURE_WALL};
  _sim.addMaterial(class_name, Component::genName(flow_channel.name(), class_name), params);
}

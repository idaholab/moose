#include "Closures1PhaseBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowChannel.h"

template <>
InputParameters
validParams<Closures1PhaseBase>()
{
  InputParameters params = validParams<ClosuresBase>();
  return params;
}

Closures1PhaseBase::Closures1PhaseBase(const InputParameters & params) : ClosuresBase(params) {}

void
Closures1PhaseBase::addWallFrictionFunctionMaterial(const FlowChannel & flow_channel) const
{
  const FunctionName & f_D_fn_name = flow_channel.getParam<FunctionName>("f");
  flow_channel.makeFunctionControllableIfConstant(f_D_fn_name, "f");

  const std::string class_name = "WallFrictionFunctionMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<MaterialPropertyName>("f_D") = FlowModelSinglePhase::FRICTION_FACTOR_DARCY;
  params.set<FunctionName>("function") = f_D_fn_name;
  params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
  params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
  params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
  _sim.addMaterial(class_name, Component::genName(flow_channel.name(), class_name), params);
}

void
Closures1PhaseBase::addAverageWallTemperatureMaterial(const FlowChannel & flow_channel) const
{
  const std::string class_name = "AverageWallTemperature3EqnMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<std::vector<VariableName>>("T_wall_sources") = flow_channel.getWallTemperatureNames();
  params.set<std::vector<MaterialPropertyName>>("Hw_sources") =
      flow_channel.getWallHTCNames1Phase();
  params.set<std::vector<VariableName>>("P_hf_sources") = flow_channel.getHeatedPerimeterNames();
  params.set<std::vector<VariableName>>("P_hf_total") = {FlowModel::HEAT_FLUX_PERIMETER};
  params.set<MaterialPropertyName>("Hw_average") =
      FlowModelSinglePhase::HEAT_TRANSFER_COEFFICIENT_WALL;
  params.set<std::vector<VariableName>>("T_fluid") = {FlowModelSinglePhase::TEMPERATURE};
  _sim.addMaterial(class_name, Component::genName(flow_channel.name(), class_name), params);
}

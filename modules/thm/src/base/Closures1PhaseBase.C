#include "Closures1PhaseBase.h"
#include "FlowModelSinglePhase.h"
#include "Pipe.h"

template <>
InputParameters
validParams<Closures1PhaseBase>()
{
  InputParameters params = validParams<ClosuresBase>();
  return params;
}

Closures1PhaseBase::Closures1PhaseBase(const InputParameters & params) : ClosuresBase(params) {}

void
Closures1PhaseBase::addWallFrictionFunctionMaterial(const Pipe & flow_channel) const
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

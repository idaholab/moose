#include "FormLossFromFunction1Phase.h"
#include "FlowChannel1Phase.h"
#include "THMApp.h"

registerMooseObject("THMApp", FormLossFromFunction1Phase);

template <>
InputParameters
validParams<FormLossFromFunction1Phase>()
{
  InputParameters params = validParams<Component>();
  params.addRequiredParam<std::string>(
      "flow_channel", "The name of the flow channel where form loss is will be applied");
  params.addRequiredParam<FunctionName>("K_prime",
                                        "Form loss coefficient per unit length function");

  params.addClassDescription(
      "Prescribe a form loss over a 1-phase flow channel given by a function");

  return params;
}

FormLossFromFunction1Phase::FormLossFromFunction1Phase(const InputParameters & params)
  : Component(params), _flow_channel_name(getParam<std::string>("flow_channel"))
{
}

void
FormLossFromFunction1Phase::check() const
{
  checkComponentOfTypeExistsByName<FlowChannel1Phase>(_flow_channel_name);
}

void
FormLossFromFunction1Phase::addMooseObjects()
{
  const FlowChannel1Phase & fch = getComponentByName<FlowChannel1Phase>(_flow_channel_name);

  const std::string class_name = "OneD3EqnMomentumFormLoss";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
  params.set<std::vector<SubdomainName>>("block") = fch.getSubdomainNames();
  params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
  params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
  params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
  params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
  params.set<MaterialPropertyName>("rho") = FlowModelSinglePhase::DENSITY;
  params.set<MaterialPropertyName>("vel") = FlowModelSinglePhase::VELOCITY;
  params.set<FunctionName>("K_prime") = getParam<FunctionName>("K_prime");
  _sim.addKernel(class_name, Component::genName(name(), class_name), params);
}

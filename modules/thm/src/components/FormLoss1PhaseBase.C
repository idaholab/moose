#include "FormLoss1PhaseBase.h"
#include "FlowModelSinglePhase.h"
#include "FlowChannel1Phase.h"

template <>
InputParameters
validParams<FormLoss1PhaseBase>()
{
  InputParameters params = validParams<Component>();
  params.addRequiredParam<std::string>(
      "flow_channel", "The name of the flow channel where form loss is will be applied");
  params.addClassDescription("Base class for prescribing a form loss over a 1-phase flow channel");

  return params;
}

FormLoss1PhaseBase::FormLoss1PhaseBase(const InputParameters & params)
  : Component(params), _flow_channel_name(getParam<std::string>("flow_channel"))
{
}

void
FormLoss1PhaseBase::init()
{
  Component::init();

  if (hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const FlowChannel1Phase & fch = getComponentByName<FlowChannel1Phase>(_flow_channel_name);
    _flow_channel_subdomains = fch.getSubdomainNames();
  }
}

void
FormLoss1PhaseBase::check() const
{
  checkComponentOfTypeExistsByName<FlowChannel1Phase>(_flow_channel_name);
}

void
FormLoss1PhaseBase::addMooseObjects()
{
  {
    const std::string class_name = "OneD3EqnMomentumFormLoss";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<NonlinearVariableName>("variable") = FlowModelSinglePhase::RHOUA;
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<std::vector<VariableName>>("arhoA") = {FlowModelSinglePhase::RHOA};
    params.set<std::vector<VariableName>>("arhouA") = {FlowModelSinglePhase::RHOUA};
    params.set<std::vector<VariableName>>("arhoEA") = {FlowModelSinglePhase::RHOEA};
    params.set<std::vector<VariableName>>("A") = {FlowModel::AREA};
    params.set<MaterialPropertyName>("rho") = FlowModelSinglePhase::DENSITY;
    params.set<MaterialPropertyName>("vel") = FlowModelSinglePhase::VELOCITY;
    _sim.addKernel(class_name, genName(name(), FlowModelSinglePhase::RHOUA, "form_loss"), params);
  }
}

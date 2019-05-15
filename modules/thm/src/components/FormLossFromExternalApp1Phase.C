#include "FormLossFromExternalApp1Phase.h"
#include "FlowChannel1Phase.h"
#include "THMApp.h"

registerMooseObject("THMApp", FormLossFromExternalApp1Phase);

template <>
InputParameters
validParams<FormLossFromExternalApp1Phase>()
{
  InputParameters params = validParams<Component>();
  params.addRequiredParam<std::string>(
      "flow_channel", "The name of the flow channel where form loss is will be applied");

  params.addClassDescription("Apply a distributed form loss over a 1-phase flow channel computed "
                             "by an external application.");

  return params;
}

FormLossFromExternalApp1Phase::FormLossFromExternalApp1Phase(const InputParameters & params)
  : Component(params),
    _K_prime_var_name("K_prime"),
    _flow_channel_name(getParam<std::string>("flow_channel"))
{
}

void
FormLossFromExternalApp1Phase::init()
{
  Component::init();

  if (hasComponentByName<FlowChannel1Phase>(_flow_channel_name))
  {
    const FlowChannel1Phase & fch = getComponentByName<FlowChannel1Phase>(_flow_channel_name);
    _block_ids_flow_channel = fch.getSubdomainIds();
    _flow_channel_subdomains = fch.getSubdomainNames();
  }
}

void
FormLossFromExternalApp1Phase::check() const
{
  Component::check();

  checkComponentOfTypeExistsByName<FlowChannel1Phase>(_flow_channel_name);
}

void
FormLossFromExternalApp1Phase::addVariables()
{
  _sim.addVariable(false, _K_prime_var_name, FEType(FIRST, LAGRANGE), _block_ids_flow_channel);
}

void
FormLossFromExternalApp1Phase::addMooseObjects()
{
  {
    const std::string class_name = "CoupledVariableValueMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<MaterialPropertyName>("prop_name") = "K_prime";
    params.set<std::vector<VariableName>>("coupled_variable") = {_K_prime_var_name};
    _sim.addMaterial(class_name, Component::genName(name(), "k_prime_material"), params);
  }
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
    _sim.addKernel(
        class_name, Component::genName(name(), FlowModelSinglePhase::RHOUA, "form_loss"), params);
  }
}

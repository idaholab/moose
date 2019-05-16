#include "FormLossFromExternalApp1Phase.h"

registerMooseObject("THMApp", FormLossFromExternalApp1Phase);

template <>
InputParameters
validParams<FormLossFromExternalApp1Phase>()
{
  InputParameters params = validParams<FormLoss1PhaseBase>();

  params.addClassDescription("Apply a distributed form loss over a 1-phase flow channel computed "
                             "by an external application.");

  return params;
}

FormLossFromExternalApp1Phase::FormLossFromExternalApp1Phase(const InputParameters & params)
  : FormLoss1PhaseBase(params), _K_prime_var_name("K_prime")
{
}

void
FormLossFromExternalApp1Phase::addVariables()
{
  _sim.addVariable(false, _K_prime_var_name, FEType(FIRST, LAGRANGE), _block_ids_flow_channel);
}

void
FormLossFromExternalApp1Phase::addMooseObjects()
{
  FormLoss1PhaseBase::addMooseObjects();

  {
    const std::string class_name = "CoupledVariableValueMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<MaterialPropertyName>("prop_name") = "K_prime";
    params.set<std::vector<VariableName>>("coupled_variable") = {_K_prime_var_name};
    _sim.addMaterial(class_name, Component::genName(name(), "k_prime_material"), params);
  }
}

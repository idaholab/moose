#include "FormLossFromFunction1Phase.h"

registerMooseObject("THMApp", FormLossFromFunction1Phase);

template <>
InputParameters
validParams<FormLossFromFunction1Phase>()
{
  InputParameters params = validParams<FormLoss1PhaseBase>();
  params.addRequiredParam<FunctionName>("K_prime",
                                        "Form loss coefficient per unit length function");

  params.addClassDescription(
      "Prescribe a form loss over a 1-phase flow channel given by a function");

  return params;
}

FormLossFromFunction1Phase::FormLossFromFunction1Phase(const InputParameters & params)
  : FormLoss1PhaseBase(params)
{
}

void
FormLossFromFunction1Phase::addMooseObjects()
{
  FormLoss1PhaseBase::addMooseObjects();

  {
    const std::string class_name = "GenericFunctionMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = _flow_channel_subdomains;
    params.set<std::vector<std::string>>("prop_names") = {"K_prime"};
    params.set<std::vector<FunctionName>>("prop_values") = {getParam<FunctionName>("K_prime")};
    _sim.addMaterial(class_name, Component::genName(name(), "k_prime_material"), params);
  }
}

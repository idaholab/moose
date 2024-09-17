//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ClosuresBase.h"
#include "FlowChannelBase.h"

InputParameters
ClosuresBase::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.addPrivateParam<THMProblem *>("_thm_problem");
  params.addPrivateParam<Logger *>("_logger");
  params.addParam<bool>(
      "add_regular_materials",
      true,
      "Whether to create the regular material properties version of the closure properties");
  params.addParam<bool>(
      "add_functor_materials", false, "Whether to create the corresponding functor materials");
  params.addParam<std::vector<OutputName>>(
      "outputs", {"none"}, "Output for the material properties");

  params.registerBase("Closures");

  return params;
}

ClosuresBase::ClosuresBase(const InputParameters & params)
  : MooseObject(params),
    LoggingInterface(*params.getCheckedPointerParam<Logger *>("_logger")),
    NamingInterface(),

    _sim(*params.getCheckedPointerParam<THMProblem *>("_thm_problem")),
    _factory(_app.getFactory()),
    _add_regular_materials(getParam<bool>("add_regular_materials")),
    _add_functor_materials(getParam<bool>("add_functor_materials"))
{
}

void
ClosuresBase::addZeroMaterial(const FlowChannelBase & flow_channel,
                              const std::string & property_name) const
{
  if (_add_regular_materials)
  {
    const std::string class_name = "ADConstantMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<std::string>("property_name") = property_name;
    params.set<Real>("value") = 0;
    _sim.addMaterial(class_name, genName(flow_channel.name(), "const_mat", property_name), params);
  }
}

void
ClosuresBase::addWeightedAverageMaterial(const FlowChannelBase & flow_channel,
                                         const std::vector<MaterialPropertyName> & values,
                                         const std::vector<VariableName> & weights,
                                         const MaterialPropertyName & property_name) const
{
  if (_add_regular_materials)
  {
    const std::string class_name = "ADWeightedAverageMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<MaterialPropertyName>("prop_name") = property_name;
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<std::vector<MaterialPropertyName>>("values") = values;
    params.set<std::vector<VariableName>>("weights") = weights;
    _sim.addMaterial(class_name, genName(flow_channel.name(), "wavg_mat", property_name), params);
  }

  if (_add_functor_materials)
  {
    // Convert to MooseFunctorName as well
    std::string expression = "";
    std::vector<std::string> functors;
    for (const auto i : index_range(weights))
    {
      expression += (i == 0 ? " + " : "") + weights[i] + " * " + values[i];
      functors.push_back(weights[i]);
      functors.push_back(values[i]);
    }
    const std::string class_name = "ADParsedFunctorMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<std::string>("property_name") = property_name;
    params.set<std::string>("expression") = expression;
    params.set<std::vector<std::string>>("functor_names") = functors;
    _sim.addFunctorMaterial(class_name, genName(flow_channel.name(), "T_wall_mat"), params);
  }
}

void
ClosuresBase::addWallTemperatureFromAuxMaterial(const FlowChannelBase & flow_channel,
                                                unsigned int i) const
{
  if (_add_regular_materials)
  {
    const std::string class_name = "ADCoupledVariableValueMaterial";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
    params.set<MaterialPropertyName>("prop_name") = {flow_channel.getWallTemperatureNames()[i]};
    params.set<std::vector<VariableName>>("coupled_variable") = {
        flow_channel.getWallTemperatureNames()[i]};
    _sim.addMaterial(class_name, genName(flow_channel.name(), "coupled_var_mat", i), params);
  }
}

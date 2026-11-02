//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ClosuresBase.h"
#include "FlowChannelClosuresInterface.h"
#include "FEProblemBase.h"

InputParameters
ClosuresBase::validParams()
{
  InputParameters params = MooseObject::validParams();

  params.addPrivateParam<FEProblemBase *>("_problem");
  params.addPrivateParam<Logger *>("_logger");

  params.registerBase("Closures");

  return params;
}

ClosuresBase::ClosuresBase(const InputParameters & params)
  : MooseObject(params),
    LoggingInterface(*params.getCheckedPointerParam<Logger *>("_logger")),
    NamingInterface(),

    _problem(*params.getCheckedPointerParam<FEProblemBase *>("_problem")),
    _factory(_app.getFactory())
{
}

void
ClosuresBase::addZeroMaterial(const FlowChannelClosuresInterface & flow_channel,
                              const std::string & property_name) const
{
  const std::string class_name = "ADConstantMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getClosuresBlocks();
  params.set<std::string>("property_name") = property_name;
  params.set<Real>("value") = 0;
  _problem.addMaterial(
      class_name, genName(flow_channel.getClosuresName(), "const_mat", property_name), params);
}

void
ClosuresBase::addWeightedAverageMaterial(const FlowChannelClosuresInterface & flow_channel,
                                         const std::vector<MaterialPropertyName> & values,
                                         const std::vector<VariableName> & weights,
                                         const MaterialPropertyName & property_name) const
{
  const std::string class_name = "ADWeightedAverageMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<MaterialPropertyName>("prop_name") = property_name;
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getClosuresBlocks();
  params.set<std::vector<MaterialPropertyName>>("values") = values;
  params.set<std::vector<VariableName>>("weights") = weights;
  _problem.addMaterial(
      class_name, genName(flow_channel.getClosuresName(), "wavg_mat", property_name), params);
}

void
ClosuresBase::addWallTemperatureFromAuxMaterial(const FlowChannelClosuresInterface & flow_channel,
                                                unsigned int i) const
{
  const std::string class_name = "ADCoupledVariableValueMaterial";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getClosuresBlocks();
  params.set<MaterialPropertyName>("prop_name") = {
      flow_channel.getClosuresWallTemperatureNames()[i]};
  params.set<std::vector<VariableName>>("coupled_variable") = {
      flow_channel.getClosuresWallTemperatureNames()[i]};
  _problem.addMaterial(
      class_name, genName(flow_channel.getClosuresName(), "coupled_var_mat", i), params);
}

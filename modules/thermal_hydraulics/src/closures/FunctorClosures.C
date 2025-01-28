//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctorClosures.h"
#include "FlowChannelBase.h"

registerMooseObject("ThermalHydraulicsApp", FunctorClosures);

InputParameters
FunctorClosures::validParams()
{
  InputParameters params = ClosuresBase::validParams();

  params.addRequiredParam<std::vector<MooseFunctorName>>(
      "functors",
      "The functors defining each material property. Each entry is paired with the corresponding "
      "entry in 'properties'.");
  params.addRequiredParam<std::vector<MaterialPropertyName>>("properties",
                                                             "The material properties to create");

  params.addClassDescription("Creates arbitrary closures from functors.");

  return params;
}

FunctorClosures::FunctorClosures(const InputParameters & params) : ClosuresBase(params) {}

void
FunctorClosures::addMooseObjectsFlowChannel(const FlowChannelBase & flow_channel)
{
  const auto & functor_names = getParam<std::vector<MooseFunctorName>>("functors");
  const auto & property_names = getParam<std::vector<MaterialPropertyName>>("properties");

  if (functor_names.size() != property_names.size())
    mooseError("The parameters 'functors' and 'properties' must have the same size.");

  const std::string class_name = "MaterialFunctorConverter";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<std::vector<SubdomainName>>("block") = flow_channel.getSubdomainNames();
  params.set<std::vector<MooseFunctorName>>("functors_in") = functor_names;
  params.set<std::vector<MaterialPropertyName>>("ad_props_out") = property_names;
  _sim.addMaterial(class_name, genName(flow_channel.name(), name()), params);
}

void
FunctorClosures::addMooseObjectsHeatTransfer(const HeatTransferBase & /*heat_transfer*/,
                                             const FlowChannelBase & /*flow_channel*/)
{
}

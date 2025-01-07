//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarTransferBase.h"
#include "FlowChannelBase.h"
#include "ClosuresBase.h"
#include "MooseUtils.h"

InputParameters
ScalarTransferBase::validParams()
{
  InputParameters params = ConnectorBase::validParams();
  params.addRequiredParam<std::string>("flow_channel",
                                       "Name of flow channel component to connect to");
  params.addRequiredParam<std::vector<VariableName>>(
      "passive_scalar_names", "Vector containing the names of the advected scalar variables.");
  return params;
}

ScalarTransferBase::ScalarTransferBase(const InputParameters & parameters)
  : ConnectorBase(parameters),
    _flow_channel_name(getParam<std::string>("flow_channel")),
    _passive_scalar_names(getParam<std::vector<VariableName>>("passive_scalar_names"))
{
  addDependency(_flow_channel_name);
}

void
ScalarTransferBase::init()
{
  ConnectorBase::init();

  checkComponentOfTypeExistsByName<FlowChannelBase>(_flow_channel_name);

  if (hasComponentByName<FlowChannelBase>(_flow_channel_name))
  {
    const FlowChannelBase & flow_channel = getComponentByName<FlowChannelBase>(_flow_channel_name);

    // add the name of this scalar transfer component to list for flow channel
    flow_channel.addScalarTransferName(name());

    // get various data from flow channel
    _flow_channel_subdomains = flow_channel.getSubdomainNames();
    _fp_name = flow_channel.getFluidPropertiesName();
    _A_fn_name = flow_channel.getAreaFunctionName();
    _closures = flow_channel.getClosures();
  }
}

void
ScalarTransferBase::initSecondary()
{
  ConnectorBase::initSecondary();

  // determine names of scalar transfer functors
  if (hasComponentByName<FlowChannelBase>(_flow_channel_name))
  {
    const FlowChannelBase & flow_channel = getComponentByName<FlowChannelBase>(_flow_channel_name);

    const std::string suffix = flow_channel.getScalarTransferNamesSuffix(name());

    for (const auto & scalar_name : _passive_scalar_names)
      _wall_scalar_flux_names.push_back(scalar_name + suffix);
  }
}

void
ScalarTransferBase::check() const
{
  ConnectorBase::check();
}

const MooseFunctorName &
ScalarTransferBase::getWallScalarFluxName(unsigned int scalar_i) const
{
  return _wall_scalar_flux_names[scalar_i];
}

const UserObjectName &
ScalarTransferBase::getFluidPropertiesName() const
{
  checkSetupStatus(INITIALIZED_PRIMARY);

  return _fp_name;
}

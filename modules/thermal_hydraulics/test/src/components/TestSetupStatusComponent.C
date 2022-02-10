//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestSetupStatusComponent.h"
#include "FlowChannelBase.h"

registerMooseObject("ThermalHydraulicsTestApp", TestSetupStatusComponent);

InputParameters
TestSetupStatusComponent::validParams()
{
  InputParameters params = Component::validParams();

  params.addRequiredParam<std::string>("flow_channel", "Name of flow channel to use for test");

  return params;
}

TestSetupStatusComponent::TestSetupStatusComponent(const InputParameters & params)
  : Component(params)
{
}

void
TestSetupStatusComponent::init()
{
  const FlowChannelBase & flow_channel = getComponent<FlowChannelBase>("flow_channel");

  // This call should throw an error because FlowChannelBase initializes the data required
  // by this call in its init() function, which due to the ordering in the
  // test input file, should not have been called at this point.
  flow_channel.getFlowModel();
}

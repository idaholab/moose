#include "TestSetupStatusComponent.h"
#include "FlowChannelBase.h"

registerMooseObject("THMTestApp", TestSetupStatusComponent);

template <>
InputParameters
validParams<TestSetupStatusComponent>()
{
  InputParameters params = validParams<Component>();

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
  flow_channel.getFlowModelID();
}

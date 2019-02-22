#include "TestSetupStatusComponent.h"
#include "Pipe.h"

registerMooseObject("RELAP7TestApp", TestSetupStatusComponent);

template <>
InputParameters
validParams<TestSetupStatusComponent>()
{
  InputParameters params = validParams<Component>();

  params.addRequiredParam<std::string>("pipe", "Name of pipe to use for test");

  return params;
}

TestSetupStatusComponent::TestSetupStatusComponent(const InputParameters & params)
  : Component(params)
{
}

void
TestSetupStatusComponent::init()
{
  const Pipe & pipe = getComponent<Pipe>("pipe");

  // This call should throw an error because Pipe initializes the data required
  // by this call in its init() function, which due to the ordering in the
  // test input file, should not have been called at this point.
  pipe.getFlowModelID();
}

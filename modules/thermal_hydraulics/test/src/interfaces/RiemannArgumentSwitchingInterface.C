#include "RiemannArgumentSwitchingInterface.h"
#include "MooseObject.h"

InputParameters
RiemannArgumentSwitchingInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addRequiredParam<bool>("switch_left_and_right", "Switch the left and right arguments?");

  return params;
}

RiemannArgumentSwitchingInterface::RiemannArgumentSwitchingInterface(
    const MooseObject * moose_object)
  : _switch_left_and_right(moose_object->getParam<bool>("switch_left_and_right"))
{
}

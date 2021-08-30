#include "ADShaftComponentTorqueScalarKernel.h"
#include "UserObject.h"
#include "ADShaftConnectableUserObjectInterface.h"

registerMooseObject("THMApp", ADShaftComponentTorqueScalarKernel);

InputParameters
ADShaftComponentTorqueScalarKernel::validParams()
{
  InputParameters params = ADScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("shaft_connected_component_uo",
                                          "Shaft connected component user object name");
  params.addClassDescription("Torque contributed by a component connected to a shaft");
  return params;
}

ADShaftComponentTorqueScalarKernel::ADShaftComponentTorqueScalarKernel(
    const InputParameters & parameters)
  : ADScalarKernel(parameters),
    _shaft_connected_component_uo(
        getUserObject<ADShaftConnectableUserObjectInterface>("shaft_connected_component_uo"))
{
}

ADReal
ADShaftComponentTorqueScalarKernel::computeQpResidual()
{
  return -_shaft_connected_component_uo.getTorque();
}

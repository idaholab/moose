#include "Turbine1PhaseFrictionTorqueAux.h"
#include "ShaftConnectedTurbine1PhaseUserObject.h"

registerMooseObject("THMApp", Turbine1PhaseFrictionTorqueAux);

InputParameters
Turbine1PhaseFrictionTorqueAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription("Friction torque computed in the 1-phase shaft-connected turbine.");
  return params;
}

Turbine1PhaseFrictionTorqueAux::Turbine1PhaseFrictionTorqueAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ShaftConnectedTurbine1PhaseUserObject>("turbine_uo"))
{
}

Real
Turbine1PhaseFrictionTorqueAux::computeValue()
{
  return _turbine_uo.getFrictionTorque();
}

#include "Turbine1PhaseDrivingTorqueAux.h"
#include "ShaftConnectedTurbine1PhaseUserObject.h"

registerMooseObject("THMApp", Turbine1PhaseDrivingTorqueAux);

InputParameters
Turbine1PhaseDrivingTorqueAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription("Driving torque computed in the 1-phase shaft-connected turbine.");
  return params;
}

Turbine1PhaseDrivingTorqueAux::Turbine1PhaseDrivingTorqueAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ShaftConnectedTurbine1PhaseUserObject>("turbine_uo"))
{
}

Real
Turbine1PhaseDrivingTorqueAux::computeValue()
{
  return _turbine_uo.getDrivingTorque();
}

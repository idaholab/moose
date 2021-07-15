#include "Turbine1PhaseDrivingTorqueAux.h"
#include "ADShaftConnectedTurbine1PhaseUserObject.h"

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
    _turbine_uo(getUserObject<ADShaftConnectedTurbine1PhaseUserObject>("turbine_uo"))
{
}

Real
Turbine1PhaseDrivingTorqueAux::computeValue()
{
  return MetaPhysicL::raw_value(_turbine_uo.getDrivingTorque());
}

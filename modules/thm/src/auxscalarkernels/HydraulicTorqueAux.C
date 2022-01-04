#include "HydraulicTorqueAux.h"
#include "ADShaftConnectedPump1PhaseUserObject.h"

registerMooseObject("THMApp", HydraulicTorqueAux);

InputParameters
HydraulicTorqueAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("pump_uo", "Pump user object name");
  params.addClassDescription("Hydraulic torque computed in the 1-phase shaft-connected pump.");
  return params;
}

HydraulicTorqueAux::HydraulicTorqueAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _pump_uo(getUserObject<ADShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

Real
HydraulicTorqueAux::computeValue()
{
  return MetaPhysicL::raw_value(_pump_uo.getHydraulicTorque());
}

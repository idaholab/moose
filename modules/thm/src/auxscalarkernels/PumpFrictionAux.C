#include "PumpFrictionAux.h"
#include "ShaftConnectedPump1PhaseUserObject.h"

registerMooseObject("THMApp", PumpFrictionAux);

InputParameters
PumpFrictionAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("pump_uo", "Pump user object name");
  params.addClassDescription("Friction torque computed in the 1-phase shaft-connected pump.");
  return params;
}

PumpFrictionAux::PumpFrictionAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _pump_uo(getUserObject<ShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

Real
PumpFrictionAux::computeValue()
{
  return _pump_uo.getFrictionTorque();
}

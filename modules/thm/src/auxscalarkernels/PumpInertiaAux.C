#include "PumpInertiaAux.h"
#include "ShaftConnectedPump1PhaseUserObject.h"

registerMooseObject("THMApp", PumpInertiaAux);

InputParameters
PumpInertiaAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("pump_uo", "Pump user object name");
  params.addClassDescription("Moment of inertia computed in the 1-phase shaft-connected pump.");
  return params;
}

PumpInertiaAux::PumpInertiaAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _pump_uo(getUserObject<ShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

Real
PumpInertiaAux::computeValue()
{
  return _pump_uo.momentOfInertia();
}

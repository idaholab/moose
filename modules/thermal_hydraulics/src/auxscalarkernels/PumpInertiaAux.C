#include "PumpInertiaAux.h"
#include "ADShaftConnectedPump1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", PumpInertiaAux);

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
    _pump_uo(getUserObject<ADShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

Real
PumpInertiaAux::computeValue()
{
  return MetaPhysicL::raw_value(_pump_uo.getMomentOfInertia());
}

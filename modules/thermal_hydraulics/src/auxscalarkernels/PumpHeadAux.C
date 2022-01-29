#include "PumpHeadAux.h"
#include "ADShaftConnectedPump1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", PumpHeadAux);

InputParameters
PumpHeadAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("pump_uo", "Pump user object name");
  params.addClassDescription("Head computed in the 1-phase shaft-connected pump.");
  return params;
}

PumpHeadAux::PumpHeadAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _pump_uo(getUserObject<ADShaftConnectedPump1PhaseUserObject>("pump_uo"))
{
}

Real
PumpHeadAux::computeValue()
{
  return MetaPhysicL::raw_value(_pump_uo.getPumpHead());
}

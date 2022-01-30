#include "Turbine1PhasePowerAux.h"
#include "ADShaftConnectedTurbine1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", Turbine1PhasePowerAux);

InputParameters
Turbine1PhasePowerAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription("Change in pressure computed in the 1-phase shaft-connected turbine.");
  return params;
}

Turbine1PhasePowerAux::Turbine1PhasePowerAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ADShaftConnectedTurbine1PhaseUserObject>("turbine_uo"))
{
}

Real
Turbine1PhasePowerAux::computeValue()
{
  return MetaPhysicL::raw_value(_turbine_uo.getTurbinePower());
}

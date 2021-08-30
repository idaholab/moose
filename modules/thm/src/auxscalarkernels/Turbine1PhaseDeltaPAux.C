#include "Turbine1PhaseDeltaPAux.h"
#include "ADShaftConnectedTurbine1PhaseUserObject.h"

registerMooseObject("THMApp", Turbine1PhaseDeltaPAux);

InputParameters
Turbine1PhaseDeltaPAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription("Change in pressure computed in the 1-phase shaft-connected turbine.");
  return params;
}

Turbine1PhaseDeltaPAux::Turbine1PhaseDeltaPAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ADShaftConnectedTurbine1PhaseUserObject>("turbine_uo"))
{
}

Real
Turbine1PhaseDeltaPAux::computeValue()
{
  return MetaPhysicL::raw_value(_turbine_uo.getTurbineDeltaP());
}

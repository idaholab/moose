#include "Turbine1PhaseMomentOfInertiaAux.h"
#include "ShaftConnectedTurbine1PhaseUserObject.h"

registerMooseObject("THMApp", Turbine1PhaseMomentOfInertiaAux);

InputParameters
Turbine1PhaseMomentOfInertiaAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("turbine_uo", "Turbine user object name");
  params.addClassDescription("Moment of inertia computed in the 1-phase shaft-connected turbine.");
  return params;
}

Turbine1PhaseMomentOfInertiaAux::Turbine1PhaseMomentOfInertiaAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _turbine_uo(getUserObject<ShaftConnectedTurbine1PhaseUserObject>("turbine_uo"))
{
}

Real
Turbine1PhaseMomentOfInertiaAux::computeValue()
{
  return _turbine_uo.getMomentOfInertia();
}

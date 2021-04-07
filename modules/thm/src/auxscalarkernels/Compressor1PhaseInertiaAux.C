#include "Compressor1PhaseInertiaAux.h"
#include "ShaftConnectedCompressor1PhaseUserObject.h"

registerMooseObject("THMApp", Compressor1PhaseInertiaAux);

InputParameters
Compressor1PhaseInertiaAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("compressor_uo", "Compressor user object name");
  params.addClassDescription(
      "Moment of inertia computed in the 1-phase shaft-connected compressor.");
  return params;
}

Compressor1PhaseInertiaAux::Compressor1PhaseInertiaAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _compressor_uo(getUserObject<ShaftConnectedCompressor1PhaseUserObject>("compressor_uo"))
{
}

Real
Compressor1PhaseInertiaAux::computeValue()
{
  return _compressor_uo.getMomentOfInertia();
}

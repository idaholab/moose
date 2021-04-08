#include "Compressor1PhaseFrictionAux.h"
#include "ShaftConnectedCompressor1PhaseUserObject.h"

registerMooseObject("THMApp", Compressor1PhaseFrictionAux);

InputParameters
Compressor1PhaseFrictionAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("compressor_uo", "Compressor user object name");
  params.addClassDescription("Friction torque computed in the 1-phase shaft-connected compressor.");
  return params;
}

Compressor1PhaseFrictionAux::Compressor1PhaseFrictionAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _compressor_uo(getUserObject<ShaftConnectedCompressor1PhaseUserObject>("compressor_uo"))
{
}

Real
Compressor1PhaseFrictionAux::computeValue()
{
  return _compressor_uo.getFrictionTorque();
}

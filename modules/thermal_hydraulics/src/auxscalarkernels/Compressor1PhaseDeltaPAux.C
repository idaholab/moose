#include "Compressor1PhaseDeltaPAux.h"
#include "ADShaftConnectedCompressor1PhaseUserObject.h"

registerMooseObject("ThermalHydraulicsApp", Compressor1PhaseDeltaPAux);

InputParameters
Compressor1PhaseDeltaPAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("compressor_uo", "Compressor user object name");
  params.addClassDescription(
      "Change in pressure computed in the 1-phase shaft-connected compressor.");
  return params;
}

Compressor1PhaseDeltaPAux::Compressor1PhaseDeltaPAux(const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _compressor_uo(getUserObject<ADShaftConnectedCompressor1PhaseUserObject>("compressor_uo"))
{
}

Real
Compressor1PhaseDeltaPAux::computeValue()
{
  return MetaPhysicL::raw_value(_compressor_uo.getCompressorDeltaP());
}

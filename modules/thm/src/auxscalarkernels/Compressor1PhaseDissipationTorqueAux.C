#include "Compressor1PhaseDissipationTorqueAux.h"
#include "ADShaftConnectedCompressor1PhaseUserObject.h"

registerMooseObject("THMApp", Compressor1PhaseDissipationTorqueAux);

InputParameters
Compressor1PhaseDissipationTorqueAux::validParams()
{
  InputParameters params = AuxScalarKernel::validParams();
  params.addRequiredParam<UserObjectName>("compressor_uo", "Compressor user object name");
  params.addClassDescription(
      "Dissipation torque computed in the 1-phase shaft-connected compressor.");
  return params;
}

Compressor1PhaseDissipationTorqueAux::Compressor1PhaseDissipationTorqueAux(
    const InputParameters & parameters)
  : AuxScalarKernel(parameters),
    _compressor_uo(getUserObject<ADShaftConnectedCompressor1PhaseUserObject>("compressor_uo"))
{
}

Real
Compressor1PhaseDissipationTorqueAux::computeValue()
{
  return MetaPhysicL::raw_value(_compressor_uo.getDissipationTorque());
}

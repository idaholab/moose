#include "RemappedLiquidVolumeFractionAux.h"
#include "Function.h"
#include "VolumeFractionMapper.h"

registerMooseObject("THMApp", RemappedLiquidVolumeFractionAux);

template <>
InputParameters
validParams<RemappedLiquidVolumeFractionAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription(
      "Computes the remapping of liquid volume fraction from a vapor volume fraction function");

  params.addRequiredParam<FunctionName>("alpha_vapor", "Vapor volume fraction function");
  params.addRequiredParam<UserObjectName>("vfm", "Volume fraction remapper user object name");

  return params;
}

RemappedLiquidVolumeFractionAux::RemappedLiquidVolumeFractionAux(const InputParameters & parameters)
  : AuxKernel(parameters),

    _alpha_vapor(getFunction("alpha_vapor")),
    _vfm(getUserObject<VolumeFractionMapper>("vfm"))
{
}

Real
RemappedLiquidVolumeFractionAux::computeValue()
{
  const Real alpha_liquid = 1.0 - _alpha_vapor.value(_t, _q_point[_qp]);
  return _vfm.beta(alpha_liquid);
}

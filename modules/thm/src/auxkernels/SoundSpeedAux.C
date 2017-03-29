#include "SoundSpeedAux.h"
#include "SinglePhaseFluidProperties.h"

template <>
InputParameters
validParams<SoundSpeedAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("v", "specific volume");
  params.addRequiredCoupledVar("e", "specific internal energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object to use.");

  return params;
}

SoundSpeedAux::SoundSpeedAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _v(coupledValue("v")),
    _e(coupledValue("e")),
    _fp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

SoundSpeedAux::~SoundSpeedAux() {}

Real
SoundSpeedAux::computeValue()
{
  return _fp.c(_v[_qp], _e[_qp]);
}

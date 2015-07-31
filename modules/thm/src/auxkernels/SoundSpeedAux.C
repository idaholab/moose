#include "SoundSpeedAux.h"
#include "SinglePhaseCommonFluidProperties.h"

template<>
InputParameters validParams<SoundSpeedAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "momentum");
  params.addRequiredCoupledVar("rhoE", "total energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object to use.");

  return params;
}

SoundSpeedAux::SoundSpeedAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhoE(coupledValue("rhoE")),
    _spfp(getUserObject<SinglePhaseCommonFluidProperties>("fp"))
{
}

SoundSpeedAux::~SoundSpeedAux()
{
}

Real
SoundSpeedAux::computeValue()
{
  return _spfp.c(_rho[_qp], _rhou[_qp], _rhoE[_qp]);
}

#include "SoundSpeedAux.h"
#include "SinglePhaseFluidProperties.h"

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

SoundSpeedAux::SoundSpeedAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhoE(coupledValue("rhoE")),
    _spfp(getUserObject<SinglePhaseFluidProperties>("fp"))
{
}

SoundSpeedAux::~SoundSpeedAux()
{
}

Real
SoundSpeedAux::computeValue()
{
  Real c2 = _spfp.c2(_rho[_qp], _rhou[_qp], _rhoE[_qp]);
  if (c2 < 0)
    mooseError("Sound speed went negative. Aborting...");
  return std::sqrt(c2);
}

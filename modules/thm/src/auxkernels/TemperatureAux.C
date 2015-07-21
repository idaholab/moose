#include "TemperatureAux.h"
#include "SinglePhaseCommonFluidProperties.h"

template<>
InputParameters validParams<TemperatureAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhou", "momentum");
  params.addRequiredCoupledVar("rhoE", "total energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object to use.");

  return params;
}


TemperatureAux::TemperatureAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _rhou(coupledValue("rhou")),
    _rhoE(coupledValue("rhoE")),
    _spfp(getUserObject<SinglePhaseCommonFluidProperties>("fp"))
{
}

TemperatureAux::~TemperatureAux()
{
}

Real
TemperatureAux::computeValue()
{
  return _spfp.temperature(_rho[_qp], _rhou[_qp], _rhoE[_qp]);
}

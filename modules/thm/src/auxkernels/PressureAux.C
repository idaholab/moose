#include "PressureAux.h"
#include "SinglePhaseCommonFluidProperties.h"

template<>
InputParameters validParams<PressureAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addRequiredCoupledVar("rho", "density");
  params.addCoupledVar("rhou", "momentum");
  params.addCoupledVar("rhoE", "total energy");
  params.addRequiredParam<UserObjectName>("fp", "The name of fluid properties object to use.");
  return params;
}

PressureAux::PressureAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _rhou(isCoupled("rhou") ? coupledValue("rhou") : _zero),
    _rhoE(isCoupled("rhoE") ? coupledValue("rhoE") : _zero),
    _spfp(getUserObject<SinglePhaseCommonFluidProperties>("fp"))
{
}

PressureAux::~PressureAux()
{
}

Real
PressureAux::computeValue()
{
  return _spfp.pressure(_rho[_qp], _rhou[_qp], _rhoE[_qp]);
}

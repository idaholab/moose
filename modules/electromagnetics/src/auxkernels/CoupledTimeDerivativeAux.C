#include "CoupledTimeDerivativeAux.h"

registerMooseObject("ElkApp", CoupledTimeDerivativeAux);

InputParameters
CoupledTimeDerivativeAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("AuxKernel to calculate the time derivative of a coupled variable.");
  params.addRequiredCoupledVar("coupled", "Coupled variable.");
  return params;
}

CoupledTimeDerivativeAux::CoupledTimeDerivativeAux(const InputParameters & parameters)
  : AuxKernel(parameters), _coupled_dt(coupledDot("coupled"))
{
}

Real
CoupledTimeDerivativeAux::computeValue()
{
  return _coupled_dt[_qp];
}

#include "NSVelocityAux.h"

template<>
InputParameters validParams<NSVelocityAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("rho", "Density (conserved form)");
  params.addRequiredCoupledVar("momentum", "Momentum (conserved form)");
  return params;
}

NSVelocityAux::NSVelocityAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _rho(coupledValue("rho")),
    _momentum(coupledValue("momentum"))
{
}

Real
NSVelocityAux::computeValue()
{
  return _momentum[_qp] / _rho[_qp];
}

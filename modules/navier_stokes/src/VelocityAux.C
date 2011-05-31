#include "VelocityAux.h"

template<>
InputParameters validParams<VelocityAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addCoupledVar("rho", "");
  params.addCoupledVar("momentum", "");
  return params;
}

VelocityAux::VelocityAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _rho(coupledValue("rho")),
   _momentum(coupledValue("momentum"))
{}

Real
VelocityAux::computeValue()
{
  return _momentum[_qp] / _rho[_qp];
}

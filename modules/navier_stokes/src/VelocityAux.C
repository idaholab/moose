#include "VelocityAux.h"

template<>
InputParameters validParams<VelocityAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

VelocityAux::VelocityAux(const std::string & name, InputParameters parameters)
  :AuxKernel(name, parameters),
   _p(coupledValue("p")),
   _momentum(coupledValue("momentum"))
{}

Real
VelocityAux::computeValue()
{
  return _momentum[_qp] / _p[_qp];
}

#include "VelocityAux.h"

template<>
InputParameters validParams<VelocityAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

VelocityAux::VelocityAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   _p(coupledVal("p")),
   _momentum(coupledVal("momentum"))
{}

Real
VelocityAux::computeValue()
{
  return _momentum[_qp] / _p[_qp];
}

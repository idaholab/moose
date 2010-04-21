#include "ConstantAux.h"

template<>
InputParameters validParams<ConstantAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.set<Real>("value")=0.0;
  return params;
}

ConstantAux::ConstantAux(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :AuxKernel(name, moose_system, parameters),
   _value(_parameters.get<Real>("value"))
{}


Real
ConstantAux::computeValue()
{
  return _value;
}

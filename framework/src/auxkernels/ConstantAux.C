#include "ConstantAux.h"

template<>
InputParameters validParams<ConstantAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<Real>("value", 0.0, "Some constant value that can be read from the input file");
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

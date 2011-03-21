#include "ConstantAux.h"

template<>
InputParameters validParams<ConstantAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<Real>("value", 0.0, "Some constant value that can be read from the input file");
  return params;
}

ConstantAux::ConstantAux(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _value(getParam<Real>("value"))
{
}


Real
ConstantAux::computeValue()
{
  return _value;
}

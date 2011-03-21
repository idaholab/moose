#include "ConstantIC.h"

template<>
InputParameters validParams<ConstantIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.set<Real>("value") = 0.0;
  return params;
}

ConstantIC::ConstantIC(const std::string & name, InputParameters parameters) :
    InitialCondition(name, parameters),
    _value(getParam<Real>("value"))
{
}

Real
ConstantIC::value(const Point & /*p*/)
{
  return _value;
}


#include "ConstantIC.h"

template<>
InputParameters validParams<ConstantIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.set<Real>("value") = 0.0;
  return params;
}

ConstantIC::ConstantIC(std::string name,
                       MooseSystem & moose_system,
                       InputParameters parameters)
  :InitialCondition(name, moose_system, parameters),
   _value(parameters.get<Real>("value"))
{}

Real
ConstantIC::value(const Point & p)
{
  return _value;
}

  


  

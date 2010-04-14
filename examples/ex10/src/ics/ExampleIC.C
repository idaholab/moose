#include "ExampleIC.h"

template<>
InputParameters validParams<ExampleIC>()
{
  InputParameters params;
  params.set<Real>("value") = 0.0;
  return params;
}

ExampleIC::ExampleIC(std::string name,
                     MooseSystem & moose_system,
                     InputParameters parameters)
  :InitialCondition(name, moose_system, parameters),
   _value(parameters.get<Real>("value"))
{}

Real
ExampleIC::value(const Point & p)
{
  // _value * x
  return _value*p(0);
}

  


  

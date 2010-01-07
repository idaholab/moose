#include "ExampleIC.h"

template<>
InputParameters validParams<ExampleIC>()
{
  InputParameters params;
  params.set<Real>("value") = 0.0;
  return params;
}

ExampleIC::ExampleIC(std::string name,
                       InputParameters parameters,
                       std::string var_name)
  :InitialCondition(name,parameters,var_name),
   _value(parameters.get<Real>("value"))
{}

Real
ExampleIC::value(const Point & p)
{
  // _value * x
  return _value*p(0);
}

  


  

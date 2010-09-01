#include "ExampleIC.h"

template<>
InputParameters validParams<ExampleIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addRequiredParam<Real>("value", "The value of the initial condition");
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
  /**
   * _value * x
   * The Point class is defined in libMesh.  The spacial
   * coordinates x,y,z can be accessed individually using
   * the parenthesis operator and a numeric index from 0..2
   */
  return _value*p(0);
}

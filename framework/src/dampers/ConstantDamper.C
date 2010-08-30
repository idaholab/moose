#include "ConstantDamper.h"

// Moose includes
#include "MooseSystem.h"

template<>
InputParameters validParams<ConstantDamper>()
{
  InputParameters params = validParams<Damper>();
  params.addRequiredParam<Real>("damping", "The percentage (between 0 and 1) of the newton update to take.");
  return params;
}

ConstantDamper::ConstantDamper(std::string name, MooseSystem & moose_system, InputParameters parameters)
  :Damper(name, moose_system, parameters),
   _damping(parameters.get<Real>("damping"))
{}

Real
ConstantDamper::computeQpDamping()
{
  return _damping;
}


           

#include "GravityRZ.h"

template<>
InputParameters validParams<GravityRZ>()
{
  InputParameters params = validParams<Gravity>();
  return params;
}

GravityRZ::GravityRZ(const std::string & name, InputParameters parameters)
  :Gravity(name, parameters)
{}

Real
GravityRZ::computeQpResidual()
{
  return Gravity::computeQpResidual();
}


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
  return 2 * M_PI * _q_point[_qp](0) * Gravity::computeQpResidual();
}


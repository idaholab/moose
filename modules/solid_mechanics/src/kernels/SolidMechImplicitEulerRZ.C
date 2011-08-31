#include "SolidMechImplicitEulerRZ.h"

template<>
InputParameters validParams<SolidMechImplicitEulerRZ>()
{
  InputParameters params = validParams<SolidMechImplicitEuler>();
  return params;
}

SolidMechImplicitEulerRZ::SolidMechImplicitEulerRZ(const std::string & name, InputParameters parameters)
  :SolidMechImplicitEuler(name, parameters)
{}

Real
SolidMechImplicitEulerRZ::computeQpResidual()
{
  return 2 * M_PI * _q_point[_qp](0) * SolidMechImplicitEuler::computeQpResidual();
}

Real
SolidMechImplicitEulerRZ::computeQpJacobian()
{
  return 2 * M_PI * _q_point[_qp](0) * SolidMechImplicitEuler::computeQpJacobian();
}


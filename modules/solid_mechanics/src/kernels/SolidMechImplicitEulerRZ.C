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
  return SolidMechImplicitEuler::computeQpResidual();
}

Real
SolidMechImplicitEulerRZ::computeQpJacobian()
{
  return SolidMechImplicitEuler::computeQpJacobian();
}


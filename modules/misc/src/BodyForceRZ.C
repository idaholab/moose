#include "BodyForceRZ.h"

template<>
InputParameters validParams<BodyForceRZ>()
{
  return validParams<BodyForce>();
}


Real BodyForceRZ::computeQpResidual()
{
  return BodyForce::computeQpResidual();
}


Real BodyForceRZ::computeQpJacobian()
{
  return BodyForce::computeQpJacobian();
}

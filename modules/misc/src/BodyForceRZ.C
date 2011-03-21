#include "BodyForceRZ.h"

template<>
InputParameters validParams<BodyForceRZ>()
{
  return validParams<BodyForce>();
}


Real BodyForceRZ::computeQpResidual()
{
  return 2 * M_PI * _q_point[_qp](0) * BodyForce::computeQpResidual();
}


Real BodyForceRZ::computeQpJacobian()
{
  return 2 * M_PI * _q_point[_qp](0) * BodyForce::computeQpJacobian();
}

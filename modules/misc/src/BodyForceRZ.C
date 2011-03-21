#include "BodyForceRZ.h"

template<>
InputParameters validParams<BodyForceRZ>()
{
  return validParams<BodyForce>();
}


Real BodyForceRZ::computeQpResidual()
{
  std::cout<<_q_point[_qp](0)<<std::endl;
  return 2 * M_PI * _q_point[_qp](0) * BodyForce::computeQpResidual();
}


Real BodyForceRZ::computeQpJacobian()
{
  return 2 * M_PI * _q_point[_qp](0) * BodyForce::computeQpJacobian();
}

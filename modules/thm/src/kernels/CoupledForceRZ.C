#include "CoupledForceRZ.h"

registerMooseObject("MooseApp", CoupledForceRZ);

template <>
InputParameters
validParams<CoupledForceRZ>()
{
  InputParameters params = validParams<CoupledForce>();
  params += validParams<RZSymmetry>();
  return params;
}

CoupledForceRZ::CoupledForceRZ(const InputParameters & parameters)
  : CoupledForce(parameters), RZSymmetry(parameters)
{
}

Real
CoupledForceRZ::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * CoupledForce::computeQpResidual();
}

Real
CoupledForceRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * CoupledForce::computeQpJacobian();
}

Real
CoupledForceRZ::computeQpOffDiagJacobian(unsigned int jvar)
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * CoupledForce::computeQpOffDiagJacobian(jvar);
}

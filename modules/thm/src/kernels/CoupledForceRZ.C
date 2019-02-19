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
  Real r = computeRadius(_q_point[_qp]);
  return 2 * libMesh::pi * r * CoupledForce::computeQpResidual();
}

Real
CoupledForceRZ::computeQpJacobian()
{
  Real r = computeRadius(_q_point[_qp]);
  return 2 * libMesh::pi * r * CoupledForce::computeQpJacobian();
}

Real
CoupledForceRZ::computeQpOffDiagJacobian(unsigned int jvar)
{
  Real r = computeRadius(_q_point[_qp]);
  return 2 * libMesh::pi * r * CoupledForce::computeQpOffDiagJacobian(jvar);
}

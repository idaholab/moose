#include "CoupledForceRZ.h"

registerMooseObject("ThermalHydraulicsApp", CoupledForceRZ);

InputParameters
CoupledForceRZ::validParams()
{
  InputParameters params = CoupledForce::validParams();
  params += RZSymmetry::validParams();
  return params;
}

CoupledForceRZ::CoupledForceRZ(const InputParameters & parameters)
  : CoupledForce(parameters), RZSymmetry(this, parameters)
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

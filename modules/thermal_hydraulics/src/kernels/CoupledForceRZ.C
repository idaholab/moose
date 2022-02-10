//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

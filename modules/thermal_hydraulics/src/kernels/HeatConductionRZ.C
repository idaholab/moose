//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatConductionRZ);

InputParameters
HeatConductionRZ::validParams()
{
  InputParameters params = HeatConductionKernel::validParams();
  params += RZSymmetry::validParams();
  return params;
}

HeatConductionRZ::HeatConductionRZ(const InputParameters & parameters)
  : HeatConductionKernel(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatConductionRZ::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionKernel::computeQpResidual();
}

Real
HeatConductionRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionKernel::computeQpJacobian();
}

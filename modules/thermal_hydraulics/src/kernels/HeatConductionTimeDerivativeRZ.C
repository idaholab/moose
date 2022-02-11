//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "HeatConductionTimeDerivativeRZ.h"

registerMooseObject("ThermalHydraulicsApp", HeatConductionTimeDerivativeRZ);

InputParameters
HeatConductionTimeDerivativeRZ::validParams()
{
  InputParameters params = HeatConductionTimeDerivative::validParams();
  params += RZSymmetry::validParams();
  return params;
}

HeatConductionTimeDerivativeRZ::HeatConductionTimeDerivativeRZ(const InputParameters & parameters)
  : HeatConductionTimeDerivative(parameters), RZSymmetry(this, parameters)
{
}

Real
HeatConductionTimeDerivativeRZ::computeQpResidual()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionTimeDerivative::computeQpResidual();
}

Real
HeatConductionTimeDerivativeRZ::computeQpJacobian()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * HeatConductionTimeDerivative::computeQpJacobian();
}

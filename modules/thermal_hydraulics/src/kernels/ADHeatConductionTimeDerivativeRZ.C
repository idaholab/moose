//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADHeatConductionTimeDerivativeRZ.h"

registerMooseObject("ThermalHydraulicsApp", ADHeatConductionTimeDerivativeRZ);

InputParameters
ADHeatConductionTimeDerivativeRZ::validParams()
{
  InputParameters params = ADHeatConductionTimeDerivative::validParams();
  params += RZSymmetry::validParams();
  return params;
}

ADHeatConductionTimeDerivativeRZ::ADHeatConductionTimeDerivativeRZ(
    const InputParameters & parameters)
  : ADHeatConductionTimeDerivative(parameters), RZSymmetry(this, parameters)
{
}

ADReal
ADHeatConductionTimeDerivativeRZ::precomputeQpResidual()
{
  const ADReal circumference = computeCircumference(_q_point[_qp]);
  return circumference * ADHeatConductionTimeDerivative::precomputeQpResidual();
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionSideIntegralRZ.h"

registerMooseObject("ThermalHydraulicsApp", FunctionSideIntegralRZ);

InputParameters
FunctionSideIntegralRZ::validParams()
{
  InputParameters params = FunctionSideIntegral::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Integrates a function over sides for RZ geometry modeled by XY domain");

  return params;
}

FunctionSideIntegralRZ::FunctionSideIntegralRZ(const InputParameters & parameters)
  : FunctionSideIntegral(parameters), RZSymmetry(this, parameters)
{
}

Real
FunctionSideIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * FunctionSideIntegral::computeQpIntegral();
}

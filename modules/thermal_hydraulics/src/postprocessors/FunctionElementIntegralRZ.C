//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionElementIntegralRZ.h"

registerMooseObject("ThermalHydraulicsApp", FunctionElementIntegralRZ);

InputParameters
FunctionElementIntegralRZ::validParams()
{
  InputParameters params = FunctionElementIntegral::validParams();
  params += RZSymmetry::validParams();

  params.addClassDescription(
      "Integrates a function over elements for RZ geometry modeled by XY domain");

  return params;
}

FunctionElementIntegralRZ::FunctionElementIntegralRZ(const InputParameters & parameters)
  : FunctionElementIntegral(parameters), RZSymmetry(this, parameters)
{
}

Real
FunctionElementIntegralRZ::computeQpIntegral()
{
  const Real circumference = computeCircumference(_q_point[_qp]);
  return circumference * FunctionElementIntegral::computeQpIntegral();
}

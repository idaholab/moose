//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VariableIntegralRayKernel.h"

registerMooseObject("RayTracingApp", VariableIntegralRayKernel);

InputParameters
VariableIntegralRayKernel::validParams()
{
  InputParameters params = IntegralRayKernel::validParams();

  params.addClassDescription("Integrates a Variable or AuxVariable along a Ray.");

  params.addRequiredCoupledVar("variable", "The name of the variable to integrate");

  return params;
}

VariableIntegralRayKernel::VariableIntegralRayKernel(const InputParameters & params)
  : IntegralRayKernel(params), _u(coupledValue("variable"))
{
}

Real
VariableIntegralRayKernel::computeQpIntegral()
{
  return _u[_qp];
}

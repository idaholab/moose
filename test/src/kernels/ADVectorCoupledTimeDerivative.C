//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ADVectorCoupledTimeDerivative.h"

registerMooseObject("MooseTestApp", ADVectorCoupledTimeDerivative);

InputParameters
ADVectorCoupledTimeDerivative::validParams()
{
  InputParameters params = ADVectorKernel::validParams();
  params.addClassDescription("Adds the time derivative of a coupled vector variable, "
                             "$(\\vec{\\psi}, \\partial \\vec{v} / \\partial t)$.");
  params.addRequiredCoupledVar("v", "The coupled vector variable");
  return params;
}

ADVectorCoupledTimeDerivative::ADVectorCoupledTimeDerivative(const InputParameters & parameters)
  : ADVectorKernel(parameters), _v_dot(adCoupledVectorDot("v"))
{
}

ADReal
ADVectorCoupledTimeDerivative::computeQpResidual()
{
  return _test[_i][_qp] * _v_dot[_qp];
}

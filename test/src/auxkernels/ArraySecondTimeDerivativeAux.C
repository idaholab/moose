//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ArraySecondTimeDerivativeAux.h"

registerMooseObject("MooseTestApp", ArraySecondTimeDerivativeAux);

InputParameters
ArraySecondTimeDerivativeAux::validParams()
{
  InputParameters params = ArrayAuxKernel::validParams();
  params.addClassDescription("Second time derivative of a coupled array variable.");
  params.addRequiredCoupledVar("v", "Array variable to take the second time derivative of");
  return params;
}

ArraySecondTimeDerivativeAux::ArraySecondTimeDerivativeAux(const InputParameters & parameters)
  : ArrayAuxKernel(parameters), _v_dotdot(coupledArrayDotDot("v"))
{
}

RealEigenVector
ArraySecondTimeDerivativeAux::computeValue()
{
  return _v_dotdot[_qp];
}

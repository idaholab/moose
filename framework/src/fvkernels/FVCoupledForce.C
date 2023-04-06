//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FVCoupledForce.h"

registerMooseObject("MooseApp", FVCoupledForce);

InputParameters
FVCoupledForce::validParams()
{
  InputParameters params = FVElementalKernel::validParams();

  params.addClassDescription("Implements a source term proportional to the value of a coupled "
                             "variable.");
  params.addRequiredParam<MooseFunctorName>("v", "The coupled functor which provides the force");
  params.addParam<Real>("coef", 1.0, "Coefficent multiplier for the coupled force term.");

  return params;
}

FVCoupledForce::FVCoupledForce(const InputParameters & parameters)
  : FVElementalKernel(parameters), _v(getFunctor<ADReal>("v")), _coef(getParam<Real>("coef"))
{
}

ADReal
FVCoupledForce::computeQpResidual()
{
  return -_coef * _v(makeElemArg(_current_elem), determineState());
}

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GradDiv.h"

registerMooseObject("NavierStokesApp", GradDiv);

InputParameters
GradDiv::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addRequiredCoupledVar("u", "The x-velocity");
  params.addRequiredCoupledVar("v", "The y-velocity");
  params.addRequiredParam<unsigned short>("component",
                                          "The velocity component this object is being applied to");
  params.addParam<Real>("gamma", 1, "The penalty parameter");
  params.addClassDescription("Adds grad-div stabilization for scalar field velocity component "
                             "Navier-Stokes implementations.");
  return params;
}

GradDiv::GradDiv(const InputParameters & parameters)
  : ADKernel(parameters),
    _grad_vel_x(adCoupledGradient("u")),
    _grad_vel_y(adCoupledGradient("v")),
    _comp(getParam<unsigned short>("component")),
    _matrix_only(getParam<bool>("matrix_only")),
    _gamma(getParam<Real>("gamma"))
{
}

void
GradDiv::computeResidual()
{
  if (!_matrix_only)
    ADKernel::computeResidual();
}

ADReal
GradDiv::computeQpResidual()
{
  return _gamma * (_grad_vel_x[_qp](0) + _grad_vel_y[_qp](1)) * _grad_test[_i][_qp](_comp);
}

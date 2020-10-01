//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GrainBoundaryVelocity.h"

registerMooseObject("PhaseFieldApp", GrainBoundaryVelocity);

InputParameters
GrainBoundaryVelocity::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Compute the velocity of grain boundaries.");
  params.addParam<Real>("eta_bottom", 0.20, "start point on range of eta we are interested in.");
  params.addParam<Real>("eta_top", 0.80, "end point on range of eta we are interested in.");
  params.addRequiredCoupledVarWithAutoBuild(
      "v", "var_name_base", "op_num", "Array of coupled variables");
  return params;
}

GrainBoundaryVelocity::GrainBoundaryVelocity(const InputParameters & parameters)
  : AuxKernel(parameters),
    _eta_bottom(getParam<Real>("eta_bottom")),
    _eta_top(getParam<Real>("eta_top")),
    _op_num(coupledComponents("v")),
    _eta(coupledValues("v")),
    _deta_dt(coupledDots("v")),
    _grad_eta(coupledGradients("v"))
{
}

Real
GrainBoundaryVelocity::computeValue()
{
  _value = 0.0;
  _new_val = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    // To make sure we are getting data from the range that we want
    if ((*_eta[i])[_qp] > _eta_bottom && (*_eta[i])[_qp] < _eta_top)
      _new_val = 1.0 / (*_grad_eta[i])[_qp].norm() * (*_deta_dt[i])[_qp];
    // our calculation
    if (_new_val > _value)
      _value = _new_val;
  }

  return _value;
}

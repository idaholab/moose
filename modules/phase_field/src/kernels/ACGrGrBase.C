//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACGrGrBase.h"

InputParameters
ACGrGrBase::validParams()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addRequiredCoupledVar("v",
                               "Array of coupled order parameter names for other order parameters");
  return params;
}

ACGrGrBase::ACGrGrBase(const InputParameters & parameters)
  : ACBulk<Real>(parameters),
    _op_num(coupledComponents("v")),
    _vals(coupledValues("v")),
    _vals_var(coupledIndices("v")),
    _mu(getMaterialProperty<Real>("mu"))
{
}

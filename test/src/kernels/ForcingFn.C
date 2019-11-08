//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ForcingFn.h"

// MOOSE includes
#include "MooseVariableFE.h"

registerMooseObject("MooseTestApp", ForcingFn);

InputParameters
ForcingFn::validParams()
{
  return Kernel::validParams();
}

ForcingFn::ForcingFn(const InputParameters & parameters) : Kernel(parameters) {}

Real
ForcingFn::funcValue()
{
  //  Point pt = _qrule->get_points()[_qp];
  Point pt = _q_point[_qp];

  //  return (pt(0)*pt(0) + pt(1)*pt(1));
  if (_var.number() == 0)
    return (pt(0) * pt(0) + pt(1) * pt(1));
  else
    return -4;
}

Real
ForcingFn::computeQpResidual()
{
  return -funcValue() * _test[_i][_qp];
}

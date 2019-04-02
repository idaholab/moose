//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicalContactLMTest.h"

registerADMooseObject("MooseTestApp", MechanicalContactLMTest);

defineADValidParams(MechanicalContactLMTest, MortarConstraint, );

template <ComputeStage compute_stage>
MechanicalContactLMTest<compute_stage>::MechanicalContactLMTest(const InputParameters & parameters)
  : MortarConstraint<compute_stage>(parameters)
{
}

template <ComputeStage compute_stage>
ADResidual
MechanicalContactLMTest<compute_stage>::computeQpResidual()
{
  if (_has_master)
  {
    auto gap_vec = _phys_points_master[_qp] - _phys_points_slave[_qp];
    auto gap_mag = gap_vec.norm();
    ADReal gap;
    if (gap_vec * _normals[_qp] >= 0)
      gap = gap_mag;
    else
      gap = -gap_mag;

    return _test[_i][_qp] *
           (_lambda[_qp] + gap - std::sqrt(_lambda[_qp] * _lambda[_qp] + gap * gap));
  }
  else
    return _test[_i][_qp] * _lambda[_qp];
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EqualGradientConstraint.h"
#include "SubProblem.h"
#include "FEProblem.h"

registerADMooseObject("MooseApp", EqualGradientConstraint);

defineADValidParams(EqualGradientConstraint,
                    MortarConstraint,
                    params.addRequiredParam<unsigned int>("component",
                                                          "Gradient component to constrain"););

template <ComputeStage compute_stage>
EqualGradientConstraint<compute_stage>::EqualGradientConstraint(const InputParameters & parameters)
  : MortarConstraint<compute_stage>(parameters), _component(adGetParam<unsigned int>("component"))
{
  _need_primal_gradient = true;
}

template <ComputeStage compute_stage>
ADReal
EqualGradientConstraint<compute_stage>::computeQpResidual()
{
  return (_grad_u_master[_qp](_component) - _grad_u_slave[_qp](_component)) * _test[_i][_qp];
}

template <ComputeStage compute_stage>
ADReal
EqualGradientConstraint<compute_stage>::computeQpResidualSide(Moose::ConstraintType res_type)
{
  switch (res_type)
  {
    case Moose::Master:
      return _lambda[_qp] * _grad_test_master[_i][_qp](_component);
    case Moose::Slave:
      return -_lambda[_qp] * _grad_test_slave[_i][_qp](_component);
    default:
      return 0;
  }
}

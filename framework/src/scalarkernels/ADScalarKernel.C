//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADScalarKernel.h"

#include "Assembly.h"

InputParameters
ADScalarKernel::validParams()
{
  InputParameters params = ScalarKernelBase::validParams();
  return params;
}

ADScalarKernel::ADScalarKernel(const InputParameters & parameters)
  : ScalarKernelBase(parameters), _u(_var.adSln()), _jacobian_already_computed(false)
{
}

void
ADScalarKernel::reinit()
{
  _jacobian_already_computed = false;
}

void
ADScalarKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  for (_i = 0; _i < _var.order(); _i++)
    _local_re(_i) += raw_value(computeQpResidual());

  accumulateTaggedLocalResidual();
}

void
ADScalarKernel::computeJacobian()
{
  computeADJacobian();
}

void
ADScalarKernel::computeOffDiagJacobian(unsigned int /*jvar*/)
{
  if (!_jacobian_already_computed)
  {
    computeADJacobian();
    _jacobian_already_computed = true;
  }
}

void
ADScalarKernel::computeOffDiagJacobianScalar(unsigned int /*jvar*/)
{
}

void
ADScalarKernel::computeADJacobian()
{
  if (_residuals.size() != _var.order())
    _residuals.resize(_var.order(), 0.0);
  for (_i = 0; _i < _var.order(); _i++)
    _residuals[_i] = computeQpResidual();

  addJacobian(_assembly, _residuals, _var.dofIndices(), _var.scalingFactor());
}

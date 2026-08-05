//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "ADRowDependentTestKernel.h"

registerMooseObject("MooseTestApp", ADRowDependentTestKernel);

InputParameters
ADRowDependentTestKernel::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Tests Jacobian assembly when residual rows have different AD derivative support.");
  return params;
}

ADRowDependentTestKernel::ADRowDependentTestKernel(const InputParameters & parameters)
  : ADKernel(parameters), _ad_dof_values(_var.adDofValues()), _last_jacobian_elem(nullptr)
{
}

void
ADRowDependentTestKernel::jacobianSetup()
{
  ADKernel::jacobianSetup();
  _last_jacobian_elem = nullptr;
}

void
ADRowDependentTestKernel::computeJacobian()
{
  computeResidualsForJacobian();
  addJacobianWithHeterogeneousRowSupport(_assembly, _residuals, dofIndices(), _var.scalingFactor());
}

void
ADRowDependentTestKernel::computeResidualAndJacobian()
{
  computeResidualsForJacobian();
  addResiduals(_assembly, _residuals, _var.dofIndices(), _var.scalingFactor());
  addJacobianWithHeterogeneousRowSupport(
      _assembly, _residuals, _var.dofIndices(), _var.scalingFactor());
}

void
ADRowDependentTestKernel::computeOffDiagJacobian(const unsigned int)
{
  if (_last_jacobian_elem != _current_elem)
  {
    computeJacobian();
    _last_jacobian_elem = _current_elem;
  }
}

ADReal
ADRowDependentTestKernel::computeQpResidual()
{
  return _test[_i][_qp] * _ad_dof_values[_i];
}

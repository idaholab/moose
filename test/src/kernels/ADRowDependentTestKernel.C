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
      "Tests Jacobian assembly with configurable AD derivative support across residual rows.");
  params.addParam<bool>(
      "heterogeneous_support",
      true,
      "Whether each residual row depends on a different set of degrees of freedom.");
  return params;
}

ADRowDependentTestKernel::ADRowDependentTestKernel(const InputParameters & parameters)
  : ADKernel(parameters),
    _ad_dof_values(_var.adDofValues()),
    _heterogeneous_support(getParam<bool>("heterogeneous_support"))
{
}

ADReal
ADRowDependentTestKernel::computeQpResidual()
{
  if (_heterogeneous_support)
    return _test[_i][_qp] * _ad_dof_values[_i];

  ADReal value = _ad_dof_values[_i];
  if (_i % 2)
    for (std::size_t j = 0; j < _ad_dof_values.size(); ++j)
      value += 0.1 * _ad_dof_values[j];
  else
    for (std::size_t j = _ad_dof_values.size(); j-- > 0;)
      value += 0.1 * _ad_dof_values[j];

  return _test[_i][_qp] * value;
}

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
  params.addClassDescription("Tests Jacobian assembly when residual rows have different AD "
                             "derivative support or insertion order.");
  params.addParam<bool>("reorder_equivalent_support",
                        false,
                        "Insert equivalent row derivative supports in different orders.");
  return params;
}

ADRowDependentTestKernel::ADRowDependentTestKernel(const InputParameters & parameters)
  : ADKernel(parameters),
    _ad_dof_values(_var.adDofValues()),
    _reorder_equivalent_support(getParam<bool>("reorder_equivalent_support"))
{
}

ADReal
ADRowDependentTestKernel::computeQpResidual()
{
  if (_reorder_equivalent_support)
  {
    ADReal residual = 0;
    for (const auto j : index_range(_phi))
    {
      // MetaPhysicL canonicalizes the alternating operation order to the same support.
      const auto k = _i % 2 ? _phi.size() - 1 - j : j;
      residual += _test[_i][_qp] * _phi[k][_qp] * _ad_dof_values[k];
    }
    return residual;
  }

  return _test[_i][_qp] * _ad_dof_values[_i];
}

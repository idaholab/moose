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
  params.addClassDescription("Tests Jacobian assembly with distinct AD derivative support across "
                             "residual rows.");
  return params;
}

ADRowDependentTestKernel::ADRowDependentTestKernel(const InputParameters & parameters)
  : ADKernel(parameters), _ad_dof_values(_var.adDofValues())
{
}

ADReal
ADRowDependentTestKernel::computeQpResidual()
{
  return _test[_i][_qp] * _ad_dof_values[_i];
}

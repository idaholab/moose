//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NanKernel.h"

registerMooseObject("MooseTestApp", NanKernel);

InputParameters
NanKernel::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<unsigned int>("timestep_to_nan", 1, "The timestep number to throw a nan on");

  // Parameters for testing deprecated message
  params.addDeprecatedParam<unsigned int>(
      "deprecated_default",
      100,
      "Deprecated parameter test",
      "Set this parameter to trigger a deprecated error (default)");
  params.addDeprecatedParam<unsigned int>(
      "deprecated_no_default",
      "Deprecated parameter test",
      "Set this parameter to trigger a deprecated error (no default)");
  return params;
}

NanKernel::NanKernel(const InputParameters & parameters)
  : Kernel(parameters),
    _timestep_to_nan(getParam<unsigned int>("timestep_to_nan")),
    _deprecated_default(getParam<unsigned int>("deprecated_default")),
    _deprecated_no_default(
        isParamValid("deprecated_no_default") ? getParam<unsigned int>("deprecated_no_default") : 0)
{
}

Real
NanKernel::computeQpResidual()
{
  if (static_cast<unsigned int>(_t_step) >= _timestep_to_nan)
    // TODO: Once C++11 is fully supported, add "static_assert(std::numeric_limits::has_infinity,
    // some_msg)".
    return _grad_u[_qp] * _grad_test[_i][_qp] * std::numeric_limits<Real>::infinity();
  else
    return _grad_u[_qp] * _grad_test[_i][_qp];
}

Real
NanKernel::computeQpJacobian()
{
  return _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}

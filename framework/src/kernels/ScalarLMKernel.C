//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarLMKernel.h"

registerMooseObject("MooseApp", ScalarLMKernel);
registerMooseObject("MooseApp", ADScalarLMKernel);

template <bool is_ad>
InputParameters
ScalarLMKernelTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernelScalar<is_ad>::validParams();
  params.addClassDescription("This class is used to enforce integral of phi = V_0 with a "
                             "Lagrange multiplier approach.");
  params.renameCoupledVar("scalar_variable", "kappa", "Primary coupled scalar variable");
  params.addRequiredParam<PostprocessorName>(
      "pp_name", "Name of the Postprocessor containing the volume of the domain.");
  params.addRequiredParam<Real>(
      "value", "Given (constant) which we want the integral of the solution variable to match.");

  return params;
}

template <bool is_ad>
ScalarLMKernelTempl<is_ad>::ScalarLMKernelTempl(const InputParameters & parameters)
  : GenericKernelScalar<is_ad>(parameters),
    _value(this->template getParam<Real>("value")),
    _pp_value(this->getPostprocessorValue("pp_name"))
{
}

template <bool is_ad>
GenericReal<is_ad>
ScalarLMKernelTempl<is_ad>::computeQpResidual()
{
  return _kappa[0] * _test[_i][_qp];
}

template <bool is_ad>
GenericReal<is_ad>
ScalarLMKernelTempl<is_ad>::computeScalarQpResidual()
{
  return _u[_qp] - _value / _pp_value;
}

template <bool is_ad>
Real
ScalarLMKernelTempl<is_ad>::computeScalarQpJacobian()
{
  // This function will never be called for the AD version. But because C++ does
  // not support an optional function declaration based on a template parameter,
  // we must keep this template for all cases.
  mooseAssert(
      !is_ad,
      "In ADScalarLMKernel, computeScalarQpJacobian should not be called. Check computeJacobian "
      "implementation.");
  return 0.;
}

template <bool is_ad>
Real
ScalarLMKernelTempl<is_ad>::computeQpOffDiagJacobianScalar(unsigned int svar)
{
  // This function will never be called for the AD version. But because C++ does
  // not support an optional function declaration based on a template parameter,
  // we must keep this template for all cases.
  mooseAssert(!is_ad,
              "In ADScalarLMKernel, computeQpOffDiagJacobianScalar should not be called. Check "
              "computeOffDiagJacobianScalar "
              "implementation.");
  if (svar == _kappa_var)
    return _test[_i][_qp];
  else
    return 0.;
}

template <bool is_ad>
Real
ScalarLMKernelTempl<is_ad>::computeScalarQpOffDiagJacobian(unsigned int jvar)
{
  // This function will never be called for the AD version. But because C++ does
  // not support an optional function declaration based on a template parameter,
  // we must keep this template for all cases.
  mooseAssert(!is_ad,
              "In ADScalarLMKernel, computeScalarQpOffDiagJacobian should not be called. Check "
              "computeOffDiagJacobian "
              "implementation.");
  if (jvar == _var.number())
    return _phi[_j][_qp];
  else
    return 0.;
}

template class ScalarLMKernelTempl<false>;
template class ScalarLMKernelTempl<true>;

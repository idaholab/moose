//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BodyForce.h"

// MOOSE
#include "Function.h"
#include "Assembly.h"

registerMooseObject("MooseApp", BodyForce);
registerMooseObject("MooseApp", ADBodyForce);

template <bool is_ad>
InputParameters
BodyForceTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addClassDescription("Demonstrates the multiple ways that scalar values can be introduced "
                             "into kernels, e.g. (controllable) constants, functions, and "
                             "postprocessors. Implements the weak form $(\\psi_i, -f)$.");
  params.addParam<Real>("value", 1.0, "Coefficient to multiply by the body force term");
  params.addParam<FunctionName>("function", "1", "A function that describes the body force");
  params.addParam<PostprocessorName>(
      "postprocessor", 1, "A postprocessor whose value is multiplied by the body force");
  params.declareControllable("value");
  return params;
}

template <bool is_ad>
BodyForceTempl<is_ad>::BodyForceTempl(const InputParameters & parameters)
  : GenericKernel<is_ad>(parameters),
    _scale(this->template getParam<Real>("value")),
    _function(getFunction("function")),
    _postprocessor(getPostprocessorValue("postprocessor")),
    _generic_q_point(this->_use_displaced_mesh ? &this->_assembly.template genericQPoints<is_ad>()
                                               : nullptr)
{
}

template <bool is_ad>
GenericReal<is_ad>
BodyForceTempl<is_ad>::computeQpResidual()
{
  if (_generic_q_point)
    return -_test[_i][_qp] * _scale * _postprocessor *
           _function.value(_t, (*_generic_q_point)[_qp]);
  else
    return -_test[_i][_qp] * _scale * _postprocessor * _function.value(_t, _q_point[_qp]);
}

template class BodyForceTempl<false>;
template class BodyForceTempl<true>;

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalizationAux.h"

registerMooseObject("MooseApp", NormalizationAux);

InputParameters
NormalizationAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Normalizes a variable based on a Postprocessor value.");
  params.addRequiredCoupledVar("source_variable", "The variable to be normalized");
  params.addParam<PostprocessorName>("normalization", "The postprocessor on the source");
  params.addParam<PostprocessorName>("shift", "The postprocessor to shift the source");
  params.addParam<Real>("normal_factor", 1.0, "The normalization factor");
  return params;
}

NormalizationAux::NormalizationAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _src(coupledValue("source_variable")),
    _pp_on_source(isParamValid("normalization") ? &getPostprocessorValue("normalization") : NULL),
    _shift(isParamValid("shift") ? &getPostprocessorValue("shift") : NULL),
    _normal_factor(getParam<Real>("normal_factor"))
{
}

Real
NormalizationAux::computeValue()
{
  Real denominator = _pp_on_source ? *_pp_on_source : 1.0;
  mooseAssert(denominator != 0., "postprocessor value is zero");
  Real shift = _shift ? *_shift : 0.0;
  return _src[_qp] * _normal_factor / denominator - shift;
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NormalizationAux.h"

template <>
InputParameters
validParams<NormalizationAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredCoupledVar("source_variable", "The variable to be normalized");
  params.addRequiredParam<PostprocessorName>("normalization", "The postprocessor on the source");
  params.addParam<Real>("normal_factor", 1.0, "The normalization factor");
  return params;
}

NormalizationAux::NormalizationAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _src(coupledValue("source_variable")),
    _pp_on_source(getPostprocessorValue("normalization")),
    _normal_factor(getParam<Real>("normal_factor"))
{
}

Real
NormalizationAux::computeValue()
{
  return _src[_qp] * _normal_factor / _pp_on_source;
}

/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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

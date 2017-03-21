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

#include "ScalePostprocessor.h"

template <>
InputParameters
validParams<ScalePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("value", "The postprocessor to be scaled");
  params.addParam<Real>("scaling_factor", 1.0, "The scaling factor");
  return params;
}

ScalePostprocessor::ScalePostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _value(getPostprocessorValue("value")),
    _scaling_factor(getParam<Real>("scaling_factor"))
{
}

PostprocessorValue
ScalePostprocessor::getValue()
{
  return _scaling_factor * _value;
}

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

#include "ChangeOverTimestepPostprocessor.h"

template<>
InputParameters validParams<ChangeOverTimestepPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<PostprocessorName>("postprocessor", "The name of the postprocessor");
  return params;
}

ChangeOverTimestepPostprocessor::ChangeOverTimestepPostprocessor(const InputParameters & parameters) :
    GeneralPostprocessor(parameters),
    _pps_value(getPostprocessorValue("postprocessor")),
    _pps_value_old(getPostprocessorValueOld("postprocessor"))
{
}

void
ChangeOverTimestepPostprocessor::initialize()
{
}

void
ChangeOverTimestepPostprocessor::execute()
{
}

Real
ChangeOverTimestepPostprocessor::getValue()
{
  return _pps_value - _pps_value_old;
}

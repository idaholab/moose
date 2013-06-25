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

#include "TotalVariableValue.h"

template<>
InputParameters validParams<TotalVariableValue>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addParam<PostprocessorName>("value", "The name of the postprocessor");
  return params;
}

TotalVariableValue::TotalVariableValue(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _value(0),
    _value_old(getPostprocessorValueOldByName(_name)),
    _pps_value(getPostprocessorValue("value")),
    _pps_value_old(getPostprocessorValueOld("value"))
{
}

TotalVariableValue::~TotalVariableValue()
{
}

void
TotalVariableValue::initialize()
{
}

void
TotalVariableValue::execute()
{
  _value = _value_old  + 0.5 * (_pps_value + _pps_value_old) * _dt;
}

Real
TotalVariableValue::getValue()
{
  return _value;
}

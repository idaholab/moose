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
  params.addParam<std::string>("value", "The name of the postprocessor");
  return params;
}

TotalVariableValue::TotalVariableValue(const std::string & name, InputParameters parameters) :
    GeneralPostprocessor(name, parameters),
    _value(0),
    _pps_value(getPostprocessorValue(getParam<std::string>("value"))),
    _pps_value_old(getPostprocessorValueOld(getParam<std::string>("value")))
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
  _value += 0.5 * (_pps_value + _pps_value_old) * _dt;
}

Real
TotalVariableValue::getValue()
{
  return _value;
}

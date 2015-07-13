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

#include "TimePostprocessor.h"

template<>
InputParameters validParams<TimePostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  return params;
}

TimePostprocessor::TimePostprocessor(const std::string & name, InputParameters parameters) :
  GeneralPostprocessor(name, parameters)
{
}

PostprocessorValue
TimePostprocessor::getValue()
{
  return _t + _app.getGlobalTimeOffset();
}

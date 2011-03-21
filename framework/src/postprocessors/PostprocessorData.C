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

#include "PostprocessorData.h"

PostprocessorData::PostprocessorData()
{}

bool
PostprocessorData::hasPostprocessor(const std::string & name)
{
  return (_values.find(name) != _values.end());
}

PostprocessorValue &
PostprocessorData::getPostprocessorValue(const std::string & name)
{
  // TODO: do something smarter so we can have lazy binding like this... but still have good errors.
  /*
  if (_values.find(name) == _values.end())
    mooseError("No Data found for name: " + name);
  */
  
  return _values[name];
}

void
PostprocessorData::init(const std::string & name)
{
  _values[name] = 0.0;
}

void
PostprocessorData::storeValue(const std::string & name, PostprocessorValue value)
{
  _values[name] = value;
}

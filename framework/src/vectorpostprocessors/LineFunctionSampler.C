//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LineFunctionSampler.h"

// MOOSE includes
#include "Function.h"
#include "LineValueSampler.h"

registerMooseObject("MooseApp", LineFunctionSampler);

InputParameters
LineFunctionSampler::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Sample one or more functions along a line.");
  params += SamplerBase::validParams();

  params.addRequiredParam<Point>("start_point", "The beginning of the line");
  params.addRequiredParam<Point>("end_point", "The ending of the line");

  params.addRequiredParam<unsigned int>("num_points",
                                        "The number of points to sample along the line");

  params.addRequiredParam<std::vector<FunctionName>>("functions",
                                                     "The Functions to sample along the line");

  return params;
}

LineFunctionSampler::LineFunctionSampler(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerBase(parameters, this, _communicator),
    _start_point(getParam<Point>("start_point")),
    _end_point(getParam<Point>("end_point")),
    _num_points(getParam<unsigned int>("num_points")),
    _function_names(getParam<std::vector<FunctionName>>("functions")),
    _num_funcs(_function_names.size()),
    _functions(_num_funcs),
    _values(_num_funcs)
{
  // Get the Functions
  for (unsigned int i = 0; i < _num_funcs; i++)
    _functions[i] = &getFunctionByName(_function_names[i]);

  // Unfortunately, std::vector<FunctionName> can't be cast to std::vector<std::string>...
  std::vector<std::string> function_name_strings(_num_funcs);
  for (unsigned int i = 0; i < _num_funcs; i++)
    function_name_strings[i] = _function_names[i];

  // Initialize the datastructions in SamplerBase
  SamplerBase::setupVariables(function_name_strings);

  // Generate points along the line
  LineValueSampler::generatePointsAndIDs(_start_point, _end_point, _num_points, _points, _ids);
}

void
LineFunctionSampler::initialize()
{
  SamplerBase::initialize();
}

void
LineFunctionSampler::execute()
{
  if (processor_id() == 0) // Only sample on processor zero for now
  {
    // TODO: Thread this when we finally move to C++11
    for (unsigned int p = 0; p < _num_points; p++)
    {
      for (unsigned int i = 0; i < _num_funcs; i++)
        _values[i] = _functions[i]->value(_t, _points[p]);

      SamplerBase::addSample(_points[p], _ids[p], _values);
    }
  }
}

void
LineFunctionSampler::finalize()
{
  SamplerBase::finalize();
}

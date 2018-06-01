//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphTime.h"
#include "SubProblem.h"

#include "libmesh/system.h"

registerMooseObject("MooseApp", PerfGraphTime);

template <>
InputParameters
validParams<PerfGraphTime>()
{
  InputParameters params = validParams<GeneralPostprocessor>();

  MooseEnum time_type("SELF CHILDREN TOTAL");

  params.addRequiredParam<std::string>("section_name", "The name of the section to get time for");

  params.addRequiredParam<MooseEnum>(
      "time_type", time_type, "The type of time to retrieve for the section_name");

  params.addClassDescription("Retrieves timing information from the PerfGraph.");
  return params;
}

PerfGraphTime::PerfGraphTime(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _time(_perf_graph.getTime((PerfGraph::TimeType)(int)getParam<MooseEnum>("time_type"),
                              getParam<std::string>("section_name")))
{
}

Real
PerfGraphTime::getValue()
{
  _perf_graph.updateTiming();

  return _time;
}

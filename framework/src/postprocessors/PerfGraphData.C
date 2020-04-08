//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphData.h"
#include "SubProblem.h"

#include "libmesh/system.h"

registerMooseObject("MooseApp", PerfGraphData);

defineLegacyParams(PerfGraphData);

InputParameters
PerfGraphData::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();

  MooseEnum data_type("SELF CHILDREN TOTAL SELF_AVG CHILDREN_AVG TOTAL_AVG SELF_PERCENT "
                      "CHILDREN_PERCENT TOTAL_PERCENT CALLS");

  params.addRequiredParam<std::string>("section_name", "The name of the section to get data for");

  params.addRequiredParam<MooseEnum>(
      "data_type", data_type, "The type of data to retrieve for the section_name");

  params.addClassDescription("Retrieves timing information from the PerfGraph.");
  return params;
}

PerfGraphData::PerfGraphData(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _data_type(getParam<MooseEnum>("data_type")),
    _section_name(getParam<std::string>("section_name"))
{
}

Real
PerfGraphData::getValue()
{
  switch (_data_type)
  {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
      return _perf_graph.getTime(static_cast<PerfGraph::TimeType>(_data_type), _section_name);
    case 9:
      return _perf_graph.getNumCalls(_section_name);
  }

  mooseError("Unknown selection for data_type!");
}

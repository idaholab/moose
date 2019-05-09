//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerProcessorPerfGraphData.h"

// MOOSE includes
#include "MooseVariable.h"
#include "ThreadedElementLoopBase.h"
#include "ThreadedNodeLoop.h"

#include "libmesh/quadrature.h"

#include <numeric>

registerMooseObject("MooseApp", PerProcessorPerfGraphData);

template <>
InputParameters
validParams<PerProcessorPerfGraphData>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();

  MultiMooseEnum data_type("SELF CHILDREN TOTAL SELF_AVG CHILDREN_AVG TOTAL_AVG SELF_PERCENT "
                           "CHILDREN_PERCENT TOTAL_PERCENT CALLS");

  params.addRequiredParam<std::vector<std::string>>("section_names",
                                                    "The names of the sections to get data for");

  params.addRequiredParam<MultiMooseEnum>(
      "data_types", data_type, "The type of data to retrieve for each section_name");

  params.addClassDescription(
      "Retrieves timing information from the PerfGraph for each MPI process.  The resulting vector "
      "names will be section_name::data_type");

  return params;
}

PerProcessorPerfGraphData::PerProcessorPerfGraphData(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    _my_pid(processor_id()),
    _rank_map(_app.rankMap()),
    _my_hardware_id(_rank_map.hardwareID(processor_id())),
    _section_names(getParam<std::vector<std::string>>("section_names")),
    _pid(declareVector("pid")),
    _hardware_id(declareVector("hardware_id"))
{
  // Pull out the data_items
  const auto & data_types = getParam<MultiMooseEnum>("data_types");
  std::vector<std::string> data_type_names;

  if (data_types.size() != _section_names.size())
    paramError("data_types", "data_types size must be the same as section_names");

  _data_types.reserve(data_types.size());
  data_type_names.reserve(data_types.size());

  for (const auto & item : as_range(data_types.begin(), data_types.end()))
  {
    _data_types.push_back(static_cast<int>(item));
    data_type_names.push_back(static_cast<std::string>(item));
  }

  if (_data_types.size() != _section_names.size())
    paramError("data_types", "data_types size must match section_names size!");

  auto num_data = _data_types.size();

  _local_data.resize(num_data);
  _data_vectors.resize(num_data);

  auto num_procs = _communicator.size();

  // Declare the vectors
  for (MooseIndex(_data_types) i = 0; i < _data_types.size(); i++)
  {
    _data_vectors[i] = &declareVector(_section_names[i] + "::" + data_type_names[i]);
    if (_my_pid == 0)
      _data_vectors[i]->resize(num_procs);
  }
}

void
PerProcessorPerfGraphData::initialize()
{
}

void
PerProcessorPerfGraphData::execute()
{
  for (MooseIndex(_data_types) i = 0; i < _data_types.size(); i++)
  {
    auto data_type = _data_types[i];
    auto section_name = _section_names[i];

    switch (data_type)
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
        _local_data[i] =
            _perf_graph.getTime(static_cast<PerfGraph::TimeType>(data_type), section_name);
        break;
      case 9:
        _local_data[i] = _perf_graph.getNumCalls(section_name);
    }
  }
}

void
PerProcessorPerfGraphData::finalize()
{
  _communicator.gather(0, static_cast<Real>(_my_pid), _pid);
  _communicator.gather(0, static_cast<Real>(_my_hardware_id), _hardware_id);

  for (MooseIndex(_data_types) i = 0; i < _data_types.size(); i++)
    _communicator.gather(0, _local_data[i], *_data_vectors[i]);
}

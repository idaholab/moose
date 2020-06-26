//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphInterface.h"

#include "MooseApp.h"

defineLegacyParams(PerfGraphInterface);

InputParameters
PerfGraphInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

PerfGraphInterface::PerfGraphInterface(const MooseObject * moose_object)
  : _pg_params(&moose_object->parameters()),
    _perf_graph(
        _pg_params
            ->getCheckedPointerParam<MooseApp *>(
                "_moose_app", "PerfGraphInterface is unable to retrieve the MooseApp pointer!")
            ->perfGraph()),
    _prefix(moose_object->type())
{
}

PerfGraphInterface::PerfGraphInterface(const MooseObject * moose_object, const std::string prefix)
  : _pg_params(&moose_object->parameters()),
    _perf_graph(
        _pg_params
            ->getCheckedPointerParam<MooseApp *>(
                "_moose_app", "PerfGraphInterface is unable to retrieve the MooseApp pointer!")
            ->perfGraph()),
    _prefix(prefix)
{
}

PerfGraphInterface::PerfGraphInterface(PerfGraph & perf_graph, const std::string prefix)
  : _pg_params(nullptr), _perf_graph(perf_graph), _prefix(prefix)
{
}

PerfID
PerfGraphInterface::registerTimedSection(const std::string & section_name, const unsigned int level)
{
  if (_prefix != "")
    return _perf_graph.registerSection(_prefix + "::" + section_name, level);
  else
    return _perf_graph.registerSection(section_name, level);
}

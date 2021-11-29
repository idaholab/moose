//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphInterface.h"
#include "PerfGraphRegistry.h"

#include "MooseApp.h"

InputParameters
PerfGraphInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

PerfGraphInterface::PerfGraphInterface(const MooseObject * moose_object)
  : _pg_moose_app(*moose_object->parameters().getCheckedPointerParam<MooseApp *>(
        "_moose_app", "PerfGraphInterface is unable to retrieve the MooseApp pointer!")),
    _prefix(moose_object->type())
{
}

PerfGraphInterface::PerfGraphInterface(const MooseObject * moose_object, const std::string prefix)
  : _pg_moose_app(*moose_object->parameters().getCheckedPointerParam<MooseApp *>(
        "_moose_app", "PerfGraphInterface is unable to retrieve the MooseApp pointer!")),
    _prefix(prefix)
{
}

PerfGraphInterface::PerfGraphInterface(MooseApp & moose_app, const std::string prefix)
  : _pg_moose_app(moose_app), _prefix(prefix)
{
}

PerfGraphInterface::PerfGraphInterface(PerfGraph & perf_graph, const std::string prefix)
  : _pg_moose_app(perf_graph.mooseApp()), _prefix(prefix)
{
}

PerfID
PerfGraphInterface::registerTimedSection(const std::string & section_name,
                                         const unsigned int level) const
{
  if (_prefix != "")
    return moose::internal::getPerfGraphRegistry().registerSection(_prefix + "::" + section_name,
                                                                   level);
  else
    return moose::internal::getPerfGraphRegistry().registerSection(section_name, level);
}

PerfID
PerfGraphInterface::registerTimedSection(const std::string & section_name,
                                         const unsigned int level,
                                         const std::string & live_message,
                                         const bool print_dots) const
{
  if (_prefix != "")
    return moose::internal::getPerfGraphRegistry().registerSection(
        _prefix + "::" + section_name, level, live_message, print_dots);
  else
    return moose::internal::getPerfGraphRegistry().registerSection(
        section_name, level, live_message, print_dots);
}

PerfGraph &
PerfGraphInterface::perfGraph()
{
  return _pg_moose_app.perfGraph();
}

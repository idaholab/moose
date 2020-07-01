#include "PerfGraphRegistry.h"

namespace moose
{
namespace internal
{

PerfGraphRegistry &
getPerfGraphRegistry()
{
  static PerfGraphRegistry perf_graph_registry_singleton;

  return perf_graph_registry_singleton;
}

unsigned int
PerfGraphRegistry::registerSection(const std::string & section_name, unsigned int level)
{
  auto it = _section_name_to_id.lower_bound(section_name);

  // Is it already registered?
  if (it != _section_name_to_id.end() && it->first == section_name)
    return it->second;

  // It's not...
  auto id = _section_name_to_id.size();
  _section_name_to_id.emplace_hint(it, section_name, id);

  auto & section_info = _id_to_section_info[id];

  section_info._id = id;
  section_info._name = section_name;
  section_info._level = level;
  section_info._live_message = "";
  section_info._print_dots = false;

  return id;
}

PerfID
PerfGraphRegistry::registerSection(const std::string & section_name,
                                   unsigned int level,
                                   const std::string & live_message,
                                   const bool print_dots)
{
  if (section_name == "")
    mooseError("Section name not provided when registering timed section!");

  if (live_message == "")
    mooseError("Live message not provided when registering timed section!");

  auto it = _section_name_to_id.lower_bound(section_name);

  // Is it already registered?
  if (it != _section_name_to_id.end() && it->first == section_name)
    return it->second;

  // It's not...
  auto id = _section_name_to_id.size();
  _section_name_to_id.emplace_hint(it, section_name, id);

  auto & section_info = _id_to_section_info[id];

  section_info._id = id;
  section_info._name = section_name;
  section_info._level = level;
  section_info._live_message = live_message;
  section_info._print_dots = print_dots;

  return id;
}

}
}

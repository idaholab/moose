#include "PerfGraphRegistry.h"

#include "DataIO.h"

namespace moose
{
namespace internal
{

PerfGraphRegistry &
getPerfGraphRegistry()
{
  // In C++11 this is even thread safe! (Lookup "Static Initializers")
  static PerfGraphRegistry perf_graph_registry_singleton;

  return perf_graph_registry_singleton;
}

PerfGraphRegistry::PerfGraphRegistry()
{
  // Reserve space so that re-allocation doesn't need to happen much
  // This does not take much memory and, for most cases, will keep a single
  // reallocation from happening
  _id_to_section_info.reserve(5000);
}

unsigned int
PerfGraphRegistry::registerSection(const std::string & section_name, unsigned int level)
{
  return actuallyRegisterSection(section_name, level, "", false);
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

  return actuallyRegisterSection(section_name, level, live_message, print_dots);
}

PerfID
PerfGraphRegistry::actuallyRegisterSection(const std::string & section_name,
                                           unsigned int level,
                                           const std::string & live_message,
                                           const bool print_dots)
{
  PerfID id = 0;

  {
    std::lock_guard<std::mutex> lock(_section_name_to_id_mutex);

    auto it = _section_name_to_id.find(section_name);

    // Is it already registered?
    if (it != _section_name_to_id.end() && it->first == section_name)
      return it->second;

    // It's not...
    id = _section_name_to_id.size();

    _section_name_to_id.emplace(section_name, id);
  }

  {
    std::lock_guard<std::mutex> lock(_id_to_section_info_mutex);
    _id_to_section_info.emplace_back(id, section_name, level, live_message, print_dots);
  }

  return id;
}

PerfID
PerfGraphRegistry::sectionID(const std::string & section_name) const
{
  std::lock_guard<std::mutex> lock(_section_name_to_id_mutex);

  try
  {
    return _section_name_to_id.at(section_name);
  }
  catch (const std::out_of_range & e)
  {
    mooseError("Section Name Not Found: ", section_name);
  }
}

const PerfGraphSectionInfo &
PerfGraphRegistry::sectionInfo(const PerfID section_id) const
{
  std::lock_guard<std::mutex> lock(_id_to_section_info_mutex);

  try
  {
    return _id_to_section_info.at(section_id);
  }
  catch (const std::out_of_range & e)
  {
    mooseError("ID Not Found: ", section_id);
  }
}

bool
PerfGraphRegistry::sectionExists(const std::string & section_name) const
{
  std::lock_guard<std::mutex> lock(_section_name_to_id_mutex);

  return _section_name_to_id.count(section_name);
}

bool
PerfGraphRegistry::sectionExists(const PerfID section_id) const
{
  std::lock_guard<std::mutex> lock(_id_to_section_info_mutex);

  return section_id < _id_to_section_info.size();
}

long unsigned int
PerfGraphRegistry::numSections() const
{
  std::lock_guard<std::mutex> lock(_id_to_section_info_mutex);

  return _id_to_section_info.size();
}

const PerfGraphSectionInfo &
PerfGraphRegistry::readSectionInfo(PerfID section_id)
{
  return _id_to_section_info[section_id];
}

}
}

void
dataStore(std::ostream & stream, moose::internal::PerfGraphSectionInfo & info, void * context)
{
  dataStore(stream, info._id, context);
  dataStore(stream, info._name, context);
  dataStore(stream, info._level, context);
  dataStore(stream, info._live_message, context);
  dataStore(stream, info._print_dots, context);
}

void
dataLoad(std::istream & stream, moose::internal::PerfGraphSectionInfo & info, void * context)
{
  dataLoad(stream, info._id, context);
  dataLoad(stream, info._name, context);
  dataLoad(stream, info._level, context);
  dataLoad(stream, info._live_message, context);
  dataLoad(stream, info._print_dots, context);
}

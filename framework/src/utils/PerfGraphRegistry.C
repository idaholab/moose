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

PerfID
PerfGraphRegistry::registerTimedSection(const std::string & section_name, const unsigned int level)
{
}

PerfID
PerfGraphRegistry::registerSection(const std::string & section_name, const unsigned int level, const std::string & live_message, const bool print_dots = true)
{
}

}
}

#include "SolutionInvalidityRegistry.h"

#include "DataIO.h"

namespace moose
{
namespace internal
{

SolutionInvalidityRegistry &
getSolutionInvalidityRegistry()
{
  // In C++11 this is even thread safe! (Lookup "Static Initializers")
  static SolutionInvalidityRegistry solution_invalid_registry_singleton;

  return solution_invalid_registry_singleton;
}

SolutionInvalidityRegistry::SolutionInvalidityRegistry()
  : GeneralRegistry<std::string, SolutionInvaliditySectionInfo>("SolutionInvalidityRegistry")
{
  // Reserve space so that re-allocation doesn't need to happen much
  // This does not take much memory and, for most cases, will keep a single
  // reallocation from happening
  reserve(5000);
}

SolutionID
SolutionInvalidityRegistry::registerSection(const std::string & section_name)
{
  return actuallyRegisterSection(section_name);
}

SolutionID
SolutionInvalidityRegistry::actuallyRegisterSection(const std::string & section_name)
{
  const auto create_item = [&section_name](const std::size_t id)
  { return SolutionInvaliditySectionInfo(id, section_name); };
  return registerItem(section_name, create_item);
}

}
}

void
dataStore(std::ostream & stream,
          moose::internal::SolutionInvaliditySectionInfo & info,
          void * context)
{
}

void
dataLoad(std::istream & stream,
         moose::internal::SolutionInvaliditySectionInfo & info,
         void * context)
{
}

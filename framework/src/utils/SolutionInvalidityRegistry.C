#include "SolutionInvalidityRegistry.h"

#include "DataIO.h"

namespace moose::internal
{

SolutionInvalidityRegistry &
getSolutionInvalidityRegistry()
{
  // In C++11 this is even thread safe! (Lookup "Static Initializers")
  static SolutionInvalidityRegistry solution_invalid_registry_singleton;

  return solution_invalid_registry_singleton;
}

SolutionInvalidityRegistry::SolutionInvalidityRegistry()
  : GeneralRegistry<std::string, SolutionInvalidityInfo>("SolutionInvalidityRegistry")
{
}

InvalidSolutionID
SolutionInvalidityRegistry::registerInvalidity(const std::string & object_name,
                                               const std::string & message)
{
  const auto create_item = [&object_name, &message](const std::size_t id)
  { return SolutionInvalidityInfo(id, object_name, message); };
  return registerItem(object_name, create_item);
}

}

// std::ostream &
// operator<<(std::ostream & os, const moose::internal::SolutionInvalidityKey & key)
// {
//   os << key._name << ": " << key._message;
//   return os;
// }

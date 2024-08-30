//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  : GeneralRegistry<SolutionInvalidityName, SolutionInvalidityInfo, SoltionInvalidityNameHash>(
        "SolutionInvalidityRegistry")
{
}

InvalidSolutionID
SolutionInvalidityRegistry::registerInvalidity(const std::string & object_type,
                                               const std::string & message,
                                               const bool warning)
{
  const SolutionInvalidityName name(object_type, message);
  if (keyExists(name))
    mooseAssert(item(id(name)).warning == warning, "Inconsistent registration for a warning");
  const auto create_item = [&object_type, &message, &warning](const std::size_t id)
  { return SolutionInvalidityInfo(object_type, message, id, warning); };
  return registerItem(name, create_item);
}

std::ostream &
operator<<(std::ostream & os, const SolutionInvalidityName & name)
{
  os << name.object_type << ": " << name.message;
  return os;
}
}

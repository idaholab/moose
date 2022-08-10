//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExecFlagRegistry.h"

#include <mutex>

#include "MooseUtils.h"

namespace moose
{
namespace internal
{

ExecFlagRegistry &
getExecFlagRegistry()
{
  static ExecFlagRegistry exec_flag_registry;
  return exec_flag_registry;
}

ExecFlagRegistry::ExecFlagRegistry() {}

const ExecFlagType &
ExecFlagRegistry::registerFlag(const std::string & name)
{
  const auto name_upper = MooseUtils::toUpper(name);
  std::unique_lock lock(_flags_mutex);
  if (_flags.contains(name))
    mooseError("The exec flag ", name_upper, "is already registered");
  return _flags.addAvailableFlags(ExecFlagType(name_upper, _flags.getNextValidID()));
}

}
}

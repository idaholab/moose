//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <shared_mutex>

#include "ExecFlagEnum.h"

namespace moose
{
namespace internal
{

class ExecFlagRegistry;

/**
 * Get the global ExecFlagRegistry singleton.
 */
ExecFlagRegistry & getExecFlagRegistry();

/**
 * Registry for statically defining execute flags with consistent numbering.
 */
class ExecFlagRegistry
{
public:
  /**
   * Registers an execute flag with the given name.
   */
  const ExecFlagType & registerFlag(const std::string & name);

  /**
   * \returns The registered exec flags.
   */
  const ExecFlagEnum & getFlags() const { return _flags; };

private:
  ExecFlagRegistry();

  /// So it can be constructed
  friend ExecFlagRegistry & getExecFlagRegistry();

  /// Mutex for guarding access to _flags
  std::shared_mutex _flags_mutex;

  /// The registered flags
  ExecFlagEnum _flags;
};

}
}

#define registerExecFlag(flag) moose::internal::getExecFlagRegistry().registerFlag(flag)

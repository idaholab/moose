//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ExecFlagEnum.h"

namespace moose
{
namespace internal
{

/**
 * Registry for statically defining execute flags with consistent numbering.
 */
class ExecFlagRegistry
{
public:
  /**
   * Registers an execute flag.
   * @param name The name of the execute flag.
   * @param is_default Whether or not to define the flag as a default (available to all objects that
   * use the SetupInterface).
   */
  const ExecFlagType & registerFlag(const std::string & name, const bool is_default);

  /**
   * \returns The registered exec flags.
   */
  const ExecFlagEnum & getFlags() const { return _flags; };

  /**
   * \returns The registered default exec flags.
   */
  const ExecFlagEnum & getDefaultFlags() const { return _default_flags; };

  /// Return Singleton instance
  static ExecFlagRegistry & getExecFlagRegistry();

  ///@{ Don't allow creation through copy/move consturction or assignment
  ExecFlagRegistry(ExecFlagRegistry const &) = delete;
  ExecFlagRegistry & operator=(ExecFlagRegistry const &) = delete;

  ExecFlagRegistry(ExecFlagRegistry &&) = delete;
  ExecFlagRegistry & operator=(ExecFlagRegistry &&) = delete;
  ///@}

private:
  // Private constructor for singleton pattern
  ExecFlagRegistry() {}

  /// The registered flags
  ExecFlagEnum _flags;

  /// The default flags
  ExecFlagEnum _default_flags;
};

} // internal
} // moose

#define registerExecFlag(flag)                                                                     \
  moose::internal::ExecFlagRegistry::getExecFlagRegistry().registerFlag(flag, false)
#define registerDefaultExecFlag(flag)                                                              \
  moose::internal::ExecFlagRegistry::getExecFlagRegistry().registerFlag(flag, true)

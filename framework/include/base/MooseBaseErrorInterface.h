//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <sstream>
#include "ConsoleStreamInterface.h"
#include "MooseError.h"
#include "MooseBase.h"

/**
 * Interface that provides APIs to output errors/warnings/info messages
 */
class MooseBaseErrorInterface : public ConsoleStreamInterface
{
public:
  MooseBaseErrorInterface(const MooseBase & base, const InputParameters & params);

  /**
   * Emits an error prefixed with object name and type.
   */
  template <typename... Args>
  [[noreturn]] void mooseError(Args &&... args) const
  {
    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, std::forward<Args>(args)...);
    _moose_base.callMooseError(oss.str(), /* with_prefix = */ true);
  }

  /**
   * Emits an error without the prefixing included in mooseError().
   */
  template <typename... Args>
  [[noreturn]] void mooseErrorNonPrefixed(Args &&... args) const
  {
    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, std::forward<Args>(args)...);
    _moose_base.callMooseError(oss.str(), /* with_prefix = */ false);
  }

  /**
   * Emits a warning prefixed with object name and type.
   */
  template <typename... Args>
  void mooseWarning(Args &&... args) const
  {
    moose::internal::mooseWarningStream(
        _console, _moose_base.objectErrorPrefix("warning"), std::forward<Args>(args)...);
  }

  /**
   * Emits a warning without the prefixing included in mooseWarning().
   */
  template <typename... Args>
  void mooseWarningNonPrefixed(Args &&... args) const
  {
    moose::internal::mooseWarningStream(_console, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void mooseDeprecated(Args &&... args) const
  {
    moose::internal::mooseDeprecatedStream(_console, false, true, std::forward<Args>(args)...);
  }

  template <typename... Args>
  void mooseInfo(Args &&... args) const
  {
    moose::internal::mooseInfoStream(_console, std::forward<Args>(args)...);
  }

private:
  /// The MOOSE application this is associated with
  MooseApp & _app;

  /// The MooseBase class deriving from this interface
  const MooseBase & _moose_base;

  /// The parameters used to create this object
  const InputParameters & _params;
};

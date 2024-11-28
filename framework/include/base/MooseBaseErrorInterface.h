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
  MooseBaseErrorInterface(const MooseBase & base);

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
   * Emits a documented error with object name and type.
   *
   * Documented errors are errors that have an issue associated with them.
   *
   * The repository name \p repo_name links a named repository to a URL
   * and should be registered at the application level with registerRepository().
   * See Moose.C for an example of the "moose" repository registration.
   *
   * @param repo_name The repository name where the issue resides
   * @param issue_num The number of the issue
   * @param args The error message to be combined
   */
  template <typename... Args>
  [[noreturn]] void mooseDocumentedError(const std::string & repo_name,
                                         const unsigned int issue_num,
                                         Args &&... args) const
  {
    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, std::forward<Args>(args)...);
    const auto msg = moose::internal::formatMooseDocumentedError(repo_name, issue_num, oss.str());
    _moose_base.callMooseError(msg, /* with_prefix = */ true);
  }

  /**
   * Emits a warning prefixed with object name and type.
   */
  template <typename... Args>
  void mooseWarning(Args &&... args) const
  {
    moose::internal::mooseWarningStream(
        _console, _moose_base.errorPrefix("warning"), std::forward<Args>(args)...);
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
    moose::internal::mooseDeprecatedStream(
        _console, false, true, _moose_base.errorPrefix("deprecation"), std::forward<Args>(args)...);
  }

  template <typename... Args>
  void mooseInfo(Args &&... args) const
  {
    moose::internal::mooseInfoStream(
        _console, _moose_base.errorPrefix("information"), std::forward<Args>(args)...);
  }

private:
  /// The MooseBase class deriving from this interface
  const MooseBase & _moose_base;
};

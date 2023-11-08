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

/// Needed to break include cycle between MooseApp.h and Action.h
class MooseApp;
[[noreturn]] void callMooseErrorRaw(std::string & msg, MooseApp * app);

/**
 * Interface that provides APIs to output errors/warnings/info messages
 */
class MooseBaseErrorInterface : public ConsoleStreamInterface
{
public:
  MooseBaseErrorInterface(const MooseBase * const base)
    : ConsoleStreamInterface(base->getMooseApp()), _app(base->getMooseApp()), _moose_base(base)
  {
  }

  virtual ~MooseBaseErrorInterface() = default;

  /**
   * Emits an error prefixed with object name and type.
   */
  template <typename... Args>
  [[noreturn]] void mooseError(Args &&... args) const
  {
    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, errorPrefix("error"), std::forward<Args>(args)...);
    std::string msg = oss.str();
    callMooseErrorRaw(msg, &_app);
  }

  /**
   * Emits an error without the prefixing included in mooseError().
   */
  template <typename... Args>
  [[noreturn]] void mooseErrorNonPrefixed(Args &&... args) const
  {
    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, std::forward<Args>(args)...);
    std::string msg = oss.str();
    callMooseErrorRaw(msg, &_app);
  }

  /**
   * Emits a warning prefixed with object name and type.
   */
  template <typename... Args>
  void mooseWarning(Args &&... args) const
  {
    moose::internal::mooseWarningStream(
        _console, errorPrefix("warning"), std::forward<Args>(args)...);
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

  /**
   * A descriptive prefix for errors for this object:
   *
   * The following <error_type> occurred in the object "<name>", of type "<type>".
   */
  std::string errorPrefix(const std::string & error_type) const;

private:
  /// The MOOSE application this is associated with
  MooseApp & _app;

  /// The MooseBase class deriving from this interface
  const MooseBase * const _moose_base;
};

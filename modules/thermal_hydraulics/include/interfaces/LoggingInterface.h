//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Logger.h"

class FEProblemBase;

/**
 * Interface class for logging errors and warnings
 *
 * This allows errors/warnings to be cached and output all at once instead of
 * needing to stop each time an error is encountered.
 */
class LoggingInterface
{
public:
  /**
   * Constructor
   *
   * @param[in] fe_problem  FEProblemBase-derived object that has the Logger instance
   */
  LoggingInterface(Logger & log);

  /**
   * Logs an error
   */
  template <typename... Args>
  void logError(Args &&... args) const
  {
    _log.add(Logger::ERROR, std::forward<Args>(args)...);
  }

  /**
   * Logs an error for a component
   *
   * @param[in] component_name  Name of the component
   */
  template <typename... Args>
  void logComponentError(const std::string & component_name, Args &&... args) const
  {
    _log.add(Logger::ERROR, component_name, ": ", std::forward<Args>(args)...);
  }

  /**
   * Logs a warning
   */
  template <typename... Args>
  void logWarning(Args &&... args) const
  {
    _log.add(Logger::WARNING, std::forward<Args>(args)...);
  }

  /**
   * Logs a warning for a component
   *
   * @param[in] component_name  Name of the component
   */
  template <typename... Args>
  void logComponentWarning(const std::string & component_name, Args &&... args) const
  {
    _log.add(Logger::WARNING, component_name, ": ", std::forward<Args>(args)...);
  }

protected:
  Logger & _log;
};

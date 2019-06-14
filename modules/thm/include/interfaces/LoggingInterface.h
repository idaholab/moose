#pragma once

#include "THMApp.h"

class THMApp;
class LoggingInterface;

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
   * @param[in] app   THM application
   */
  LoggingInterface(THMApp & app);

  /**
   * Logs an error
   */
  template <typename... Args>
  void logError(Args &&... args) const
  {
    _logging_app.log().add(Logger::ERROR, std::forward<Args>(args)...);
  }

  /**
   * Logs an error for a component
   *
   * @param[in] component_name  Name of the component
   */
  template <typename... Args>
  void logComponentError(const std::string & component_name, Args &&... args) const
  {
    _logging_app.log().add(Logger::ERROR, component_name, ": ", std::forward<Args>(args)...);
  }

  /**
   * Logs a warning
   */
  template <typename... Args>
  void logWarning(Args &&... args) const
  {
    _logging_app.log().add(Logger::WARNING, std::forward<Args>(args)...);
  }

  /**
   * Logs a warning for a component
   *
   * @param[in] component_name  Name of the component
   */
  template <typename... Args>
  void logComponentWarning(const std::string & component_name, Args &&... args) const
  {
    _logging_app.log().add(Logger::WARNING, component_name, ": ", std::forward<Args>(args)...);
  }

protected:
  /// THM application
  THMApp & _logging_app;
};

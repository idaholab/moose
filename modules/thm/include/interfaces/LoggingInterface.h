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
   * @param[in] name  Name of the object performing logging
   */
  LoggingInterface(THMApp & app, const std::string & name = "");

protected:
  /**
   * Logs an error
   */
  template <typename... Args>
  void logError(Args &&... args) const
  {
    if (_logging_obj_has_name)
      _logging_app.log().add(Logger::ERROR, _logging_obj_name, ": ", std::forward<Args>(args)...);
    else
      _logging_app.log().add(Logger::ERROR, std::forward<Args>(args)...);
  }

  /**
   * Logs a warning
   */
  template <typename... Args>
  void logWarning(Args &&... args) const
  {
    if (_logging_obj_has_name)
      _logging_app.log().add(Logger::WARNING, _logging_obj_name, ": ", std::forward<Args>(args)...);
    else
      _logging_app.log().add(Logger::WARNING, std::forward<Args>(args)...);
  }

  /// THM application
  THMApp & _logging_app;

  /// Name of the object for which the error or warning is logged
  const std::string _logging_obj_name;

  /// Does the logging object have a name that should be printed?
  const bool _logging_obj_has_name;
};

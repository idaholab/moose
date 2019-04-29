#pragma once

#include "MooseError.h"

/**
 * Keeps the error and warning messages
 */
class Logger
{
public:
  Logger();
  virtual ~Logger();

  enum EMessageType
  {
    ERROR = 0,
    WARNING = 1
  };

  /**
   * Add a message to the log
   *
   * @param type The type of the message
   */
  template <typename... Args>
  void add(EMessageType type, Args &&... args)
  {
    std::ostringstream oss;
    moose::internal::mooseStreamAll(oss, args...);
    // std::string msg = mooseMsgFmt(oss.str(), "*** Info ***", COLOR_CYAN);

    EMessageType msg_type = type;
    if (type == Logger::WARNING && _warnings_are_errors)
      msg_type = Logger::ERROR;

    Logger::Message * msg = new Logger::Message(msg_type, oss.str());
    _msgs.push_back(msg);

    switch (msg_type)
    {
      case ERROR:
        _n_errors++;
        break;
      case WARNING:
        _n_warnings++;
        break;
    }
  }

  /**
   * This will print the message log
   */
  void print();

  /**
   * Is the log empty?
   *
   * @return true if there is nothing in the log, othwerwise false
   */
  bool isEmpty();

  /**
   * Return the number of errors
   *
   * @return The number of errors in this log
   */
  unsigned int getNumberOfErrors();

  /**
   * Return the number of warnings
   *
   * @return The number of warnings in this log
   */
  unsigned int getNumberOfWarnings();

  /**
   * Treat warnings as errors
   */
  void setWarningsAsErrors();

protected:
  /**
   * Simple data structure to hold the messages
   */
  class Message
  {
  public:
    Message(EMessageType type, const std::string & msg)
    {
      _type = type;
      _msg = msg;
    }

    /// The type of the message
    EMessageType _type;
    /// The text of the message
    std::string _msg;
  };

  void printMessage();

  template <typename T, typename... Args>
  void printMessage(T && val, Args &&... args)
  {
    Moose::err << val;
    printMessage(std::forward<Args>(args)...);
  }

  /// Treat warnings as errors
  bool _warnings_are_errors;
  /// The number of errors
  unsigned int _n_errors;
  /// The number of warnings
  unsigned int _n_warnings;
  /// The list of logged messages
  std::vector<Message *> _msgs;
};

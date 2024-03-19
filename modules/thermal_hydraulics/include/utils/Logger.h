//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

    Logger::Message * msg = new Logger::Message(type, oss.str());
    _msgs.push_back(msg);

    switch (type)
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
   * Calls mooseError if there are any logged errors
   */
  void emitLoggedErrors() const;

  /**
   * Calls mooseWarning if there are any logged warnings
   */
  void emitLoggedWarnings() const;

  /**
   * Return the number of errors
   *
   * @return The number of errors in this log
   */
  unsigned int getNumberOfErrors() const;

  /**
   * Return the number of warnings
   *
   * @return The number of warnings in this log
   */
  unsigned int getNumberOfWarnings() const;

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

  /// The number of errors
  unsigned int _n_errors;
  /// The number of warnings
  unsigned int _n_warnings;
  /// The list of logged messages
  std::vector<Message *> _msgs;
};

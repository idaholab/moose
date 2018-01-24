//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CONSOLESTREAM_H
#define CONSOLESTREAM_H

// C++ includes
#include <iostream>
#include <sstream>

// MOOSE includes
class OutputWarehouse;

// this is the type of s t d :: c o u t
typedef std::basic_ostream<char, std::char_traits<char>> CoutType;

// this is the function signature of std::endl
typedef CoutType & (*StandardEndLine)(CoutType &);

/**
 * A helper class for re-directing output streams to Console output objects form MooseObjects
 */
class ConsoleStream
{
public:
  /**
   * Constructor
   * @param output_warehouse A reference to the OutputWarehouse containing the Console outputs
   *
   * ConsoleStreamInterface contains an instance of this object, which allows message streams to be
   * transferred to Console output objects. This class simply provides an operator<< method
   * that passes the stream to the Console objects.
   */
  ConsoleStream(OutputWarehouse & output_warehouse);

  /**
   * The output stream operator
   * @param s The data to be output to the Console objects
   *
   * This allows any object to uses _console to write to the Console:
   *   _console << "The combination to the air lock is " << 12345 << std::endl;
   */
  template <typename StreamType>
  const ConsoleStream & operator<<(const StreamType & s) const;

  /**
   * This overload is here to handle the the std::endl manipulator
   */
  const ConsoleStream & operator<<(StandardEndLine manip) const;

private:
  /// Reference to the OutputWarhouse that contains the Console output objects
  OutputWarehouse & _output_warehouse;

  /// The stream for buffering the message
  std::ostringstream & _oss;
};

template <typename StreamType>
const ConsoleStream &
ConsoleStream::operator<<(const StreamType & s) const
{
  _oss << s;
  return *this;
}

#endif // CONSOLESTREAM_H

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
class OutputWarehouse;

// C++ includes
#include <iostream>
#include <sstream>
#include <memory>
#include <mutex>

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
  const ConsoleStream & operator<<(const StandardEndLine & manip) const;

  /**
   * Unset format flags
   */
  void unsetf(std::ios_base::fmtflags mask) const;

  std::streampos tellp() const { return _oss->tellp(); }

  /**
   * Return the current precision
   */
  std::streamsize precision() const;

  /**
   * Set the precision and return the old precision
   */
  std::streamsize precision(std::streamsize new_precision) const;

  /**
   * Return the current flags
   */
  std::ios_base::fmtflags flags() const;

  /**
   * Set the flags and return the old flags
   */
  std::ios_base::fmtflags flags(std::ios_base::fmtflags new_flags) const;

  /**
   * The number of times something has been printed
   */
  unsigned long long int numPrinted() const;

private:
  /// Reference to the OutputWarhouse that contains the Console output objects
  OutputWarehouse & _output_warehouse;

  /// The stream for buffering the message
  /// This stupidly has to be a shared pointer because
  /// of something in AutomaticMortarGeneration that requires
  /// this to be trivially copyable.
  mutable std::shared_ptr<std::ostringstream> _oss;

  /// Mutex to prevent concurrent read/writes, write/writes
  static std::mutex _stream_mutex;
};

template <typename StreamType>
const ConsoleStream &
ConsoleStream::operator<<(const StreamType & s) const
{
  std::lock_guard<std::mutex> lock(_stream_mutex);

  (*_oss) << s;
  return *this;
}

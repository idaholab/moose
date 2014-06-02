/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef CONSOLESTREAM_H
#define CONSOLESTREAM_H

// C++ includes
#include <iostream>
#include <sstream>

// MOOSE includes
#include "OutputWarehouse.h"

/**
 * A helper class for writing streams from MooseObject to Console objects
 * @see ConsoleStream
 */
class ConsoleStreamHelper
{
public:

  /**
   * Constructor
   * @see ConsoleStream
   */
  ConsoleStreamHelper(const OutputWarehouse & output_warehouse);

  /**
   * Stream output operator
   * @see ConsoleStream::operator<<
   */
  template<typename T>
  ConsoleStreamHelper & operator<<(T s);

private:

  /// Reference to the OutputWarehouse that contains Console objects
  const OutputWarehouse & _output_warehouse;

};

template<typename T>
ConsoleStreamHelper &
ConsoleStreamHelper::operator<<(T s)
{
  std::ostringstream oss;
  oss << s;
  _output_warehouse.mooseConsole(oss);
  return *this;
}

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
   * MooseObject contains an instance of this object, which allows message streams to be
   * transferred to Console output objects. This class simply provides an operator<< method
   * that passes the stream to the Console objects. Note, that this class inserts a
   * newline at the beginning of each call, but returns a reference to ConsoleStreamHelper
   * that write the stream to the Console objects with out entering a new
   */
  ConsoleStream(const OutputWarehouse & output_warehouse);

  /**
   * The output stream operator
   * @param s The data to be output to the Console objects
   *
   * This allows any MooseObject to uses _console to write to the Console:
   *   _console << "The combination to the air lock is " << 12345;
   */
  template<typename T>
  ConsoleStreamHelper & operator<<(T s);

private:

  /// Reference to the OutputWarhouse that contains the Console output objects
  const OutputWarehouse & _output_warehouse;

  /// An instance of the ConsoleStreamHelper to enable chained calls to << while inserting newline at beginning
  ConsoleStreamHelper _helper;
};

template<typename T>
ConsoleStreamHelper &
ConsoleStream::operator<<(T s)
{
  std::ostringstream oss;
  oss << std::endl << s;
  _output_warehouse.mooseConsole(oss);
  return _helper;
}

#endif // CONSOLESTREAM_H

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
#include "ConsoleStream.h"

// Forward declarations
class MooseApp;

/**
 * An inteface for the _console for outputting to the Console object
 */
class ConsoleStreamInterface
{
public:
  /**
   * A class for providing a helper stream object for writting message to
   * all the Output objects.
   */
  ConsoleStreamInterface(MooseApp & app);

  /// An instance of helper class to write streams to the Console objects
  const ConsoleStream _console;
};

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

#ifndef CONSOLESTREAMINTERFACE_H
#define CONSOLESTREAMINTERFACE_H

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

#endif // CONSOLESTREAMINTERFACE_H

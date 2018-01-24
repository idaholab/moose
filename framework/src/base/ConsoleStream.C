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

// Moose includes
#include "ConsoleStream.h"
#include "MooseUtils.h"
#include "OutputWarehouse.h"

ConsoleStream::ConsoleStream(OutputWarehouse & output_warehouse)
  : _output_warehouse(output_warehouse), _oss(output_warehouse.consoleBuffer())
{
}

const ConsoleStream & ConsoleStream::operator<<(StandardEndLine /*manip*/) const
{
  _oss << '\n';
  _output_warehouse.mooseConsole();

  return *this;
}

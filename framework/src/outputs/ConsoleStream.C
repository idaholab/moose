//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConsoleBufferTest.h"

registerMooseObject("MooseTestApp", ConsoleBufferTest);

InputParameters
ConsoleBufferTest::validParams()
{
  return GeneralUserObject::validParams();
}

ConsoleBufferTest::ConsoleBufferTest(const InputParameters & params) : GeneralUserObject(params) {}

void
ConsoleBufferTest::execute()
{
  _console << "Message that must not be retained by the console stream" << std::endl;
  if (_console.tellp() > 0)
    mooseError("The console stream still holds ", _console.tellp(), " bytes after being flushed.");
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartableData.h"

RestartableDataValue::RestartableDataValue(const std::string & name, void * const context)
  : _name(name), _context(context), _declared(false)
{
}

void
RestartableDataValue::setDeclared()
{
  mooseAssert(!_declared, "Already declared");
  _declared = true;
}

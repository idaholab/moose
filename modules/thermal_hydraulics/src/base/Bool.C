//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Bool.h"

template <>
void
dataStore(std::ostream & stream, Bool & v, void * context)
{
  dataStore(stream, v._value, context);
}

template <>
void
dataLoad(std::istream & stream, Bool & v, void * context)
{
  dataLoad(stream, v._value, context);
}

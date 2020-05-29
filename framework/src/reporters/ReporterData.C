//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterData.h"
//#include "MooseApp.h"

ReporterData::ReporterData(MooseApp & moose_app) : _app(moose_app) {}

void
ReporterData::init()
{
  // Calls the init() method of the ReporterContext objects for each Reporter value. This method
  // copies the current value to the old/older vector and shrinks this vector the the correct size
  // based on the requested old/older values.
  for (const auto & context_ptr : _context_ptrs)
    context_ptr->init();

  // Mark the data as initialized to trigger errors if calls to the declare/get methods are made.
  // This is here to help application developers avoid creating new data values in arbitrary
  // objects. It also allows for the vector storing the old/older values to be sized correctly to
  // only hold the data that is actually required for the simulation.
  _initialized = true;
}

void
ReporterData::copyValuesBack()
{
  for (const auto & context_ptr : _context_ptrs)
    context_ptr->copyValuesBack();
}

void
ReporterData::finalize(const std::string & object_name)
{
  auto func = [object_name](auto & ptr) {
    if (ptr->name().getObjectName() == object_name)
      ptr->finalize();
  };
  std::for_each(_context_ptrs.begin(), _context_ptrs.end(), func);
}

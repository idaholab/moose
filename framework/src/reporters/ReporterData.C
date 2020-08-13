//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterData.h"

ReporterData::ReporterData(MooseApp & moose_app) : _app(moose_app) {}

void
ReporterData::init()
{
  // Calls the init() method of the ReporterContext objects for each Reporter value. This method
  // copies the current value to the old/older vector and shrinks this vector the the correct size
  // based on the requested old/older values.
  for (const auto & context_ptr : _context_ptrs)
    context_ptr->init();

  // Mark the data as initialized to trigger errors if calls to the declare/get methods are made
  // after object creation. This aims to help application developers avoid creating new data values
  // in arbitrary methods. It also allows for the vector storing the old/older values to be sized
  // correctly to only hold the data that is actually required for the simulation.
  _initialized = true;

  // Create a set of values requested but not declared
  std::set<ReporterName> undeclared;
  std::set_difference(_get_names.begin(),
                      _get_names.end(),
                      _declare_names.begin(),
                      _declare_names.end(),
                      std::inserter(undeclared, undeclared.begin()));

  // Perform error checking that all gets have a declare
  if (!undeclared.empty())
  {
    std::ostringstream oss;
    oss << "The following Reporter values were not declared:";
    for (const auto & name : undeclared)
      oss << "\n    " << name;
    mooseError(oss.str());
  }
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
  // FYI, for the minimum compiler 'auto' doesn't work in argument of the lambda
  // ReporterData.C:xx:xx: error: 'auto' not allowed in lambda parameter
  auto func = [object_name](const std::unique_ptr<ReporterContextBase> & ptr) {
    if (ptr->name().getObjectName() == object_name)
      ptr->finalize();
  };
  std::for_each(_context_ptrs.begin(), _context_ptrs.end(), func);
}

void
ReporterData::store(nlohmann::json & json) const
{
  for (const auto & context_ptr : _context_ptrs)
  {
    auto & node = json.emplace_back();
    node["object_name"] = context_ptr->name().getObjectName();
    node["value_name"] = context_ptr->name().getValueName();
    node["type"] = context_ptr->type();
    context_ptr->store(node["value"]);
  }
}

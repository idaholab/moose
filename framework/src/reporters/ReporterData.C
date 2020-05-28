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
  // copyies the current value to the old/older vector and shrinks this vector the the correct size
  // based on the requested old/older values.
  for (const std::pair<std::string, std::set<RestartableDataValue *>> & data_pair : _data_ptrs)
    for (auto data_ptr : data_pair.second)
    {
      ReporterContextBase * context_ptr = static_cast<ReporterContextBase *>(data_ptr->context());
      context_ptr->init();
    }

  // Mark the data as initialized to trigger errors if calls to the declare/get methods are made.
  // This is here to help application developers avoid creating new data values in arbitrary
  // objects. It also allows for the vector storing the old/older values to be sized correctly to
  // only hold the data that is actually required for the simulation.
  _initialized = true;
}

void
ReporterData::copyValuesBack()
{
  for (const std::pair<std::string, std::set<RestartableDataValue *>> & data_pair : _data_ptrs)
    for (auto data_ptr : data_pair.second)
    {
      ReporterContextBase * context_ptr = static_cast<ReporterContextBase *>(data_ptr->context());
      context_ptr->copyValuesBack();
    }
}

void
ReporterData::finalize(const std::string & object_name)
{
  std::unordered_map<std::string, std::set<RestartableDataValue *>>::const_iterator iter =
      _data_ptrs.find(object_name);
  if (iter != _data_ptrs.end())
  {
    for (RestartableDataValue * data_ptr : iter->second)
    {
      ReporterContextBase * context_ptr = static_cast<ReporterContextBase *>(data_ptr->context());
      context_ptr->finalize();
    }
  }
}

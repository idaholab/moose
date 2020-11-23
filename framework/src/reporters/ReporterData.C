//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterData.h"
#include "MooseApp.h"

ReporterData::ReporterData(MooseApp & moose_app) : _app(moose_app) {}

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
  auto func = [object_name, this](const std::unique_ptr<ReporterContextBase> & ptr) {
    if (ptr->name().getObjectName() == object_name)
    {
      // Perform error checking for undeclared items
      if (this->_declare_names.find(ptr->name()) == this->_declare_names.end())
        mooseError("The Reporter value '", ptr->name(), "' was not declared.");

      ptr->finalize();
    }
  };
  std::for_each(_context_ptrs.begin(), _context_ptrs.end(), func);
}

void
ReporterData::store(nlohmann::json & json) const
{
  // Write information to JSON object
  for (const auto & context_ptr : _context_ptrs)
  {
    auto & node = json.emplace_back();
    node["object_name"] = context_ptr->name().getObjectName();
    node["value_name"] = context_ptr->name().getValueName();
    node["type"] = context_ptr->type();
    context_ptr->store(node["value"]);
  }
}

bool
ReporterData::hasReporterValue(const ReporterName & reporter_name) const
{
  auto func = [reporter_name](const std::unique_ptr<ReporterContextBase> & ptr) {
    return ptr->name() == reporter_name;
  };
  auto ptr = std::find_if(_context_ptrs.begin(), _context_ptrs.end(), func);
  return ptr != _context_ptrs.end();
}

std::set<ReporterName>
ReporterData::getReporterNames() const
{
  std::set<ReporterName> output;
  for (const auto & context_ptr : _context_ptrs)
    output.insert(context_ptr->name());
  return output;
}

const ReporterContextBase *
ReporterData::getReporterContextBaseHelper(const ReporterName & reporter_name) const
{
  auto func = [reporter_name](const std::unique_ptr<ReporterContextBase> & ptr) {
    return ptr->name() == reporter_name;
  };
  auto ptr = std::find_if(_context_ptrs.begin(), _context_ptrs.end(), func);
  return ptr != _context_ptrs.end() ? ptr->get() : nullptr;
}

void
ReporterData::check() const
{
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

RestartableDataValue &
ReporterData::getRestartableDataHelper(std::unique_ptr<RestartableDataValue> data_ptr,
                                       bool declare) const
{
  // get the name to avoid problems with arbitrary function argument evaluation
  std::string name = data_ptr->name();
  return _app.registerRestartableData(name, std::move(data_ptr), 0, !declare);
}

bool
ReporterData::hasReporterWithMode(const ReporterMode & mode) const
{
  for (auto & context_ptr : _context_ptrs)
  {
    if (context_ptr->getProducerModeEnum() == mode)
      return true;
  }
  return false;
}

bool
ReporterData::hasReporterWithMode(const std::string & obj_name, const ReporterMode & mode) const
{
  for (auto & context_ptr : _context_ptrs)
  {
    const ReporterName & rname = context_ptr->name();
    if (rname.getObjectName() == obj_name && context_ptr->getProducerModeEnum() == mode)
      return true;
  }
  return false;
}

void
ReporterData::transfer(const ReporterName & from_name,
                       const ReporterName & to_name,
                       ReporterData & to_data,
                       unsigned int to_index) const
{
  const ReporterContextBase * ptr = getReporterContextBaseHelper(from_name);
  if (ptr == nullptr)
    mooseError("Unable to locate Reporter with name:", from_name);
  ptr->transfer(to_data, to_name, to_index);
}

void
ReporterData::addConsumerMode(ReporterMode mode, const std::string & object_name)
{
  const ReporterContextBase * ptr = getReporterContextBaseHelper(object_name);
  if (ptr == nullptr)
    mooseError("Unable to locate Reporter with name:", object_name);
  const_cast<ReporterContextBase *>(ptr)->addConsumerMode(mode, object_name);
}

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
  for (const auto & name_context_pair : _context_ptrs)
    name_context_pair.second->copyValuesBack();
}

void
ReporterData::finalize(const std::string & object_name)
{
  for (auto & name_context_pair : _context_ptrs)
    if (name_context_pair.first.getObjectName() == object_name)
      name_context_pair.second->finalize();
}

bool
ReporterData::hasReporterValue(const ReporterName & reporter_name) const
{
  return _context_ptrs.count(reporter_name);
}

std::set<ReporterName>
ReporterData::getReporterNames() const
{
  std::set<ReporterName> output;
  for (const auto & name_context_pair : _context_ptrs)
    output.insert(name_context_pair.second->name());
  return output;
}

std::set<std::string>
ReporterData::getPostprocessorNames() const
{
  std::set<std::string> output;
  for (const auto & name_context_pair : _context_ptrs)
    if (name_context_pair.first.isPostprocessor())
      output.insert(name_context_pair.first.getObjectName());
  return output;
}

DenseVector<Real>
ReporterData::getAllRealReporterValues() const
{
  DenseVector<Real> all_values;

  std::vector<Real> & output = all_values.get_values();

  for (const auto & name_context_pair : _context_ptrs)
  {
    const ReporterName & rname = name_context_pair.first;

    if (hasReporterValue<Real>(rname))
      output.push_back(getReporterValue<Real>(rname.getCombinedName()));

    if (hasReporterValue<std::vector<Real>>(rname))
    {
      const auto & vec = getReporterValue<std::vector<Real>>(rname.getCombinedName());
      for (const auto & v : vec)
        output.push_back(v);
    }
  }

  return all_values;
}

std::vector<std::string>
ReporterData::getAllRealReporterFullNames() const
{
  std::vector<std::string> output;

  for (const auto & name_context_pair : _context_ptrs)
  {
    const ReporterName & rname = name_context_pair.first;

    if (hasReporterValue<Real>(rname))
      output.push_back(rname.getCombinedName());

    if (hasReporterValue<std::vector<Real>>(rname))
    {
      auto pname = rname.getCombinedName();
      const auto & vec = getReporterValue<std::vector<Real>>(pname);
      for (unsigned int i = 0; i < vec.size(); ++i)
        output.push_back(pname + "/" + std::to_string(i));
    }
  }

  return output;
}

const ReporterContextBase &
ReporterData::getReporterContextBase(const ReporterName & reporter_name) const
{
  if (!hasReporterValue(reporter_name))
    mooseError("Unable to locate Reporter context with name: ", reporter_name);
  return *_context_ptrs.at(reporter_name);
}

ReporterContextBase &
ReporterData::getReporterContextBase(const ReporterName & reporter_name)
{
  if (!hasReporterValue(reporter_name))
    mooseError("Unable to locate Reporter context with name: ", reporter_name);
  return *_context_ptrs.at(reporter_name);
}

const ReporterStateBase &
ReporterData::getReporterStateBase(const ReporterName & reporter_name) const
{
  if (!hasReporterState(reporter_name))
    mooseError("Unable to locate Reporter state with name: ", reporter_name);
  return *_states.at(reporter_name);
}

ReporterStateBase &
ReporterData::getReporterStateBase(const ReporterName & reporter_name)
{
  if (!hasReporterState(reporter_name))
    mooseError("Unable to locate Reporter state with name: ", reporter_name);
  return *_states.at(reporter_name);
}

void
ReporterData::check() const
{
  std::string missing;
  for (const auto & name_state_pair : _states)
    if (!hasReporterValue(name_state_pair.first))
      missing += getReporterInfo(name_state_pair.first) + "\n";

  if (missing.size())
    mooseError("The following Reporter(s) were not declared:\n\n", missing);
}

RestartableDataValue &
ReporterData::getRestartableDataHelper(std::unique_ptr<RestartableDataValue> data_ptr,
                                       bool declare) const
{
  // get the name to avoid problems with arbitrary function argument evaluation
  const auto name = data_ptr->name();
  return _app.registerRestartableData(name, std::move(data_ptr), 0, !declare);
}

bool
ReporterData::hasReporterWithMode(const std::string & obj_name, const ReporterMode & mode) const
{
  for (const auto & name_context_pair : _context_ptrs)
    if (name_context_pair.first.getObjectName() == obj_name &&
        name_context_pair.second->getProducerModeEnum() == mode)
      return true;
  return false;
}

const ReporterProducerEnum &
ReporterData::getReporterMode(const ReporterName & reporter_name) const
{
  return getReporterContextBase(reporter_name).getProducerModeEnum();
}

bool
ReporterData::hasReporterState(const ReporterName & reporter_name) const
{
  return _states.count(reporter_name);
}

std::string
ReporterData::getReporterInfo(const ReporterStateBase & state, const ReporterContextBase * context)
{
  std::stringstream oss;

  const auto & name = state.getReporterName();

  if (name.isPostprocessor())
    oss << "Postprocessor \"" << name.getObjectName() << "\":\n";
  else
  {
    oss << name.specialTypeToName() << " \"" << name.getCombinedName() << "\":\n  Type:\n    "
        << state.valueType() << "\n";
  }
  oss << "  Producer:\n    ";
  if (context)
  {
    oss << context->getProducer().type() << " \"" << context->getProducer().name() << "\"";
    oss << "\n  Context type:\n    " << context->contextType();
  }
  else
    oss << "None";
  oss << "\n  Consumer(s):\n";
  if (state.getConsumers().empty())
    oss << "    None\n";
  else
    for (const auto & mode_object_pair : state.getConsumers())
    {
      const ReporterMode mode = mode_object_pair.first;
      const MooseObject * object = mode_object_pair.second;
      oss << "    " << object->typeAndName() << " (mode: " << mode << ")\n";
    }

  return oss.str();
}

std::string
ReporterData::getReporterInfo(const ReporterName & reporter_name) const
{
  return getReporterInfo(getReporterStateBase(reporter_name),
                         hasReporterValue(reporter_name) ? &getReporterContextBase(reporter_name)
                                                         : nullptr);
}

std::string
ReporterData::getReporterInfo() const
{
  std::string out = _states.empty() ? "No reporters were requested or declared." : "";
  for (const auto & name : getReporterNames())
    out += getReporterInfo(name) + "\n";
  return out;
}

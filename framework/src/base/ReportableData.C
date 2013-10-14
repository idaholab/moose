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

#include "ReportableData.h"
#include "FEProblem.h"

ReportableData::ReportableData(FEProblem & fe_problem) :
    Restartable("values", "ReportableData", fe_problem),
    _names(std::vector<std::set<std::string> >(libMesh::n_threads(), std::set<std::string>()))
{
}

void
ReportableData::init(const std::string & name, Real value, THREAD_ID tid)
{
  // Check that the name was not already used
  if (hasReportableValue(name, tid))
    mooseError("A reportable value with the name " << name << " was already initiallized");

  // Create the current value
  _values.insert(std::pair<std::string, ReportableValue>(name, ReportableValue(value)));

  // Create the old value
  _values_old.insert(std::pair<std::string, ReportableValue>(name, ReportableValue(value)));

  // Update the name storage
  _names[tid].insert(name);
}

bool
ReportableData::hasReportableValue(const std::string & name, THREAD_ID tid)
{
  // Return true if the supplied name was already defined on the thread
  if (_names[tid].find(name) != _names[tid].end())
    return true;
  else
    return false;
}


ReportableValue &
ReportableData::getReportableValue(const std::string & name)
{
  return _values[name];
}

ReportableValue &
ReportableData::getReportableValueOld(const std::string & name)
{
  return _values_old[name];
}

void
ReportableData::storeValue(const std::string & name, Real value)
{
  // If the reportable value exists, store the given data, error if it was not found
  if (hasReportableValue(name))
    _values[name] = value;
  else
    mooseError("A ReportableValue with the name " << name << " does not exist, must initialize first");
}

const std::map<std::string, ReportableValue> &
ReportableData::values() const
{
  return _values;
}

void
ReportableData::copyValuesBack()
{
  _values_old = _values;
}

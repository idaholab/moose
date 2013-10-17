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
    _names(declareRestartableData<std::vector<std::set<std::string> > >("reportable_names", std::vector<std::set<std::string> >(libMesh::n_threads(), std::set<std::string>()))),
    _output(declareRestartableData<std::map<std::string, bool> >("reportable_output"))
{
}

void
ReportableData::init(const std::string & name, Real value, THREAD_ID tid, bool output)
{
  // Check that the name was not already used
  if (hasReportableValue(name, tid))
    mooseError("A reportable value with the name " << name << " already exists");

  // Declare the data as restartable
  if (_values.find(name) == _values.end())
  {
    _values[name] = &declareRestartableDataWithObjectName<ReportableValue>(name, "reportable_values");
    _values_old[name] = &declareRestartableDataWithObjectName<ReportableValue>(name, "reportable_values_old");
  }

  // Set the values
  getReportableValue(name) = value;
  getReportableValueOld(name) = value;

  // Output the output flag
  _output.insert(std::pair<std::string, bool>(name, output));

  // Update the name storage
  _names[tid].insert(name);
}

bool
ReportableData::hasReportableValue(const std::string & name, THREAD_ID tid)
{
  // Return true if the supplied name was already defined on the thread
  return _names[tid].find(name) != _names[tid].end();
}


ReportableValue &
ReportableData::getReportableValue(const std::string & name)
{
  // If the stored value does not exists; create it, this allows getReportableValue to be called
  // without calling init
  if (_values.find(name) == _values.end())
    _values[name] = &declareRestartableDataWithObjectName<ReportableValue>(name, "reportable_values");

  return *_values[name];
}

ReportableValue &
ReportableData::getReportableValueOld(const std::string & name)
{
  if (_values.find(name) == _values.end())
    _values_old[name] = &declareRestartableDataWithObjectName<ReportableValue>(name, "reportable_values_old");

  return *_values_old[name];
}

void
ReportableData::storeValue(const std::string & name, Real value)
{
  if (hasReportableValue(name))
    getReportableValue(name) = value;
  else
    mooseError("A ReportableValue with the name " << name << " does not exist, must initialize first");
}


const std::map<std::string, ReportableValue*> &
ReportableData::values() const
{
  return _values;
}

void
ReportableData::copyValuesBack()
{
  _values_old = _values;
}

bool
ReportableData::valueOutput(std::string name)
{
  return _output[name];
}

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

#include "Reportable.h"
#include "FEProblem.h"

Reportable::Reportable(std::string name, InputParameters & parameters) :
    _reportable_feproblem(parameters.get<FEProblem *>("_fe_problem")),
    _reportable_data(_reportable_feproblem->getReportableData()),
    _reportable_tid(parameters.isParamValid("_tid") ? parameters.get<THREAD_ID>("_tid") : 0),
    _reportable_object_name(name)
{
}

Reportable::~Reportable()
{
}

ReportableValue &
Reportable::declareReportableValue(std::string name, Real value, bool output)
{
  std::string long_name = longName(name);
  _reportable_data.init(long_name, value, _reportable_tid, output);
  return _reportable_data.getReportableValue(long_name);
}

ReportableValue &
Reportable::declareReportableValueByName(std::string name, Real value, bool output)
{
  _reportable_data.init(name, value, _reportable_tid, output);
  return _reportable_data.getReportableValue(name);
}

const ReportableValue &
Reportable::getReportableValue(std::string name)
{
  return _reportable_data.getReportableValue(longName(name));
}

const ReportableValue &
Reportable::getReportableValueByName(std::string name)
{
  return _reportable_data.getReportableValue(name);
}

bool
Reportable::hasReportableValue(std::string name)
{
  return _reportable_data.hasReportableValue(longName(name), _reportable_tid);
}

bool
Reportable::hasReportableValueByName(std::string name)
{
  return _reportable_data.hasReportableValue(name, _reportable_tid);
}

std::string
Reportable::longName(std::string & name)
{
  std::string long_name(_reportable_object_name);
  long_name += "/";
  long_name += name;
  return long_name;
}

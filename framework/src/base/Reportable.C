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
    _reportable_tid(parameters.get<THREAD_ID>("_tid"))
{
}

Reportable::~Reportable()
{
}

ReportableValue &
Reportable::declareReportableValue(std::string name, Real value)
{
  _reportable_data.init(name, value, _reportable_tid);
  return getReportableValue(name);
}

ReportableValue &
Reportable::getReportableValue(std::string name)
{
  return _reportable_data.getReportableValue(name);
}

bool
Reportable::hasReportableValue(std::string name)
{
  return _reportable_data.hasReportableValue(name, _reportable_tid);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseObject.h"
#include "ReporterInterface.h"
#include "FEProblemBase.h"
#include "DependencyResolverInterface.h"

InputParameters
ReporterInterface::validParams()
{
  return emptyInputParameters();
}

ReporterInterface::ReporterInterface(const MooseObject * moose_object)
  : _ri_params(moose_object->parameters()),
    _ri_fe_problem_base(*_ri_params.getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")),
    _ri_reporter_data(_ri_fe_problem_base.getReporterData(ReporterData::WriteKey())),
    _ri_name(moose_object->name())
{
}

bool
ReporterInterface::hasReporterValue(const std::string & param_name) const
{
  const ReporterName & reporter_name = _ri_params.template get<ReporterName>(param_name);
  return hasReporterValueByName(reporter_name);
}

bool
ReporterInterface::hasReporterValueByName(const ReporterName & reporter_name) const
{
  return _ri_reporter_data.hasReporterValue(reporter_name);
}

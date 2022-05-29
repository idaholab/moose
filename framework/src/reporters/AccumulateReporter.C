//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AccumulateReporter.h"

registerMooseObject("MooseApp", AccumulateReporter);

InputParameters
AccumulateReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter which accumulates the value of a inputted reporter value "
                             "over time into a vector reporter value of the same type.");
  params.addRequiredParam<std::vector<ReporterName>>("reporters", "The reporters to accumulate.");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
  params.suppressParameter<ExecFlagEnum>("execute_on");
  return params;
}

AccumulateReporter::AccumulateReporter(const InputParameters & parameters)
  : GeneralReporter(parameters)
{
}

void
AccumulateReporter::initialSetup()
{
  const ReporterData & rdata = _fe_problem.getReporterData();
  for (const auto & rname : getParam<std::vector<ReporterName>>("reporters"))
  {
    if (!rdata.hasReporterValue(rname))
      paramError("reporters", "Reporter ", rname, " does not exist.");

    if (!declareAccumulateHelper<int>(rname) && !declareAccumulateHelper<Real>(rname) &&
        !declareAccumulateHelper<dof_id_type>(rname) &&
        !declareAccumulateHelper<std::string>(rname) &&
        !declareAccumulateHelper<std::vector<int>>(rname) &&
        !declareAccumulateHelper<std::vector<Real>>(rname) &&
        !declareAccumulateHelper<std::vector<std::string>>(rname) &&
        !declareAccumulateHelper<std::vector<dof_id_type>>(rname))
      paramError("reporters",
                 "Reporter value ",
                 rname,
                 " is of unsupported type ",
                 rdata.getReporterContextBase(rname).type(),
                 ".");
  }
}

void
AccumulateReporter::execute()
{
  unsigned int ind = static_cast<unsigned int>(_t_step);
  for (auto & val : _accumulated_values)
    val->accumulate(ind);
}

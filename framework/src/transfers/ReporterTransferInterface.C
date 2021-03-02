//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterTransferInterface.h"

#include "UserObject.h"
#include "Reporter.h"

InputParameters
ReporterTransferInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

ReporterTransferInterface::ReporterTransferInterface(const InputParameters & /*parameters*/) {}

void
ReporterTransferInterface::addReporterTransferMode(const ReporterName & name,
                                                   const ReporterMode & mode,
                                                   FEProblemBase & problem)
{
  problem.getReporterData(ReporterData::WriteKey()).addConsumerMode(mode, name);
}

void
ReporterTransferInterface::transferReporter(const ReporterName & from_reporter,
                                            const ReporterName & to_reporter,
                                            const FEProblemBase & from_problem,
                                            FEProblemBase & to_problem,
                                            unsigned int time_index)
{
  const ReporterData & from_data = from_problem.getReporterData();
  ReporterData & to_data = to_problem.getReporterData(ReporterData::WriteKey());
  from_data.transfer(from_reporter, to_reporter, to_data, time_index);
}

void
ReporterTransferInterface::transferToVectorReporter(const ReporterName & from_reporter,
                                                    const ReporterName & to_reporter,
                                                    const FEProblemBase & from_problem,
                                                    FEProblemBase & to_problem,
                                                    dof_id_type index,
                                                    unsigned int time_index)
{
  const ReporterData & from_data = from_problem.getReporterData();
  ReporterData & to_data = to_problem.getReporterData(ReporterData::WriteKey());
  const ReporterContextBase * from_context = from_data.getReporterContextBase(from_reporter);
  from_context->transferToVector(to_data, to_reporter, index, time_index);
}

void
ReporterTransferInterface::declareClone(const ReporterName & from_reporter,
                                        const ReporterName & to_reporter,
                                        const FEProblemBase & from_problem,
                                        FEProblemBase & to_problem,
                                        const ReporterMode & mode)
{
  const ReporterData & from_data = from_problem.getReporterData();
  ReporterData & to_data = to_problem.getReporterData(ReporterData::WriteKey());
  const ReporterContextBase * from_context = from_data.getReporterContextBase(from_reporter);
  from_context->declareClone(to_data, to_reporter, mode);

  // Hide variables (if requested in parameters) if name is associated with a reporter object
  if (to_problem.hasUserObject(to_reporter.getObjectName()))
  {
    UserObject & uo = to_problem.getUserObject<UserObject>(to_reporter.getObjectName());
    Reporter * rep = dynamic_cast<Reporter *>(&uo);
    if (rep)
      rep->buildOutputHideVariableList({to_reporter.getCombinedName()});
  }
}

void
ReporterTransferInterface::declareVectorClone(const ReporterName & from_reporter,
                                              const ReporterName & to_reporter,
                                              const FEProblemBase & from_problem,
                                              FEProblemBase & to_problem,
                                              const ReporterMode & mode)
{
  const ReporterData & from_data = from_problem.getReporterData();
  ReporterData & to_data = to_problem.getReporterData(ReporterData::WriteKey());
  const ReporterContextBase * from_context = from_data.getReporterContextBase(from_reporter);
  from_context->declareVectorClone(to_data, to_reporter, mode);

  // Hide variables (if requested in parameters) if name is associated with a reporter object
  if (to_problem.hasUserObject(to_reporter.getObjectName()))
  {
    UserObject & uo = to_problem.getUserObject<UserObject>(to_reporter.getObjectName());
    Reporter * rep = dynamic_cast<Reporter *>(&uo);
    if (rep)
      rep->buildOutputHideVariableList({to_reporter.getCombinedName()});
  }
}

void
ReporterTransferInterface::resizeReporter(const ReporterName & name,
                                          FEProblemBase & problem,
                                          dof_id_type n)
{
  ReporterData & data = problem.getReporterData(ReporterData::WriteKey());
  data.resize(name, n);
}

std::vector<ReporterName>
ReporterTransferInterface::getReporterNamesHelper(std::string prefix,
                                                  const std::string & obj_name,
                                                  const std::vector<ReporterName> & rep_names)
{
  if (!prefix.empty())
    prefix += ":";
  std::vector<ReporterName> rnames;
  rnames.reserve(rep_names.size());
  for (const auto & rn : rep_names)
    rnames.emplace_back(obj_name, prefix + rn.getObjectName() + ":" + rn.getValueName());
  return rnames;
}

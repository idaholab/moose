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
#include "Transfer.h"

InputParameters
ReporterTransferInterface::validParams()
{
  InputParameters params = emptyInputParameters();
  return params;
}

ReporterTransferInterface::ReporterTransferInterface(const Transfer * transfer)
  : _rti_transfer(*transfer)
{
}

void
ReporterTransferInterface::addReporterTransferMode(const ReporterName & name,
                                                   const ReporterMode & mode,
                                                   FEProblemBase & problem)
{
  checkHasReporterValue(name, problem);
  problem.getReporterData(ReporterData::WriteKey())
      .getReporterStateBase(name)
      .addConsumer(mode, _rti_transfer);
}

void
ReporterTransferInterface::transferReporter(const ReporterName & from_reporter,
                                            const ReporterName & to_reporter,
                                            const FEProblemBase & from_problem,
                                            FEProblemBase & to_problem,
                                            unsigned int time_index)
{
  checkHasReporterValue(from_reporter, from_problem);
  checkHasReporterValue(to_reporter, to_problem);
  from_problem.getReporterData()
      .getReporterContextBase(from_reporter)
      .transfer(to_problem.getReporterData(ReporterData::WriteKey()), to_reporter, time_index);
}

void
ReporterTransferInterface::transferToVectorReporter(const ReporterName & from_reporter,
                                                    const ReporterName & to_reporter,
                                                    const FEProblemBase & from_problem,
                                                    FEProblemBase & to_problem,
                                                    dof_id_type index,
                                                    unsigned int time_index)
{
  checkHasReporterValue(from_reporter, from_problem);
  checkHasReporterValue(to_reporter, to_problem);
  from_problem.getReporterData()
      .getReporterContextBase(from_reporter)
      .transferToVector(
          to_problem.getReporterData(ReporterData::WriteKey()), to_reporter, index, time_index);
}

void
ReporterTransferInterface::transferFromVectorReporter(const ReporterName & from_reporter,
                                                      const ReporterName & to_reporter,
                                                      const FEProblemBase & from_problem,
                                                      FEProblemBase & to_problem,
                                                      dof_id_type index,
                                                      unsigned int time_index)
{
  checkHasReporterValue(from_reporter, from_problem);
  checkHasReporterValue(to_reporter, to_problem);
  from_problem.getReporterData()
      .getReporterContextBase(from_reporter)
      .transferFromVector(
          to_problem.getReporterData(ReporterData::WriteKey()), to_reporter, index, time_index);
}

void
ReporterTransferInterface::hideVariableHelper(const ReporterName & reporter,
                                              FEProblemBase & problem)
{
  if (problem.hasUserObject(reporter.getObjectName()))
  {
    Reporter * rep =
        dynamic_cast<Reporter *>(&problem.getUserObject<UserObject>(reporter.getObjectName()));
    if (rep)
      rep->buildOutputHideVariableList({reporter.getCombinedName()});
  }
}

void
ReporterTransferInterface::declareClone(const ReporterName & from_reporter,
                                        const ReporterName & to_reporter,
                                        const FEProblemBase & from_problem,
                                        FEProblemBase & to_problem,
                                        const ReporterMode & mode)
{
  checkHasReporterValue(from_reporter, from_problem);
  from_problem.getReporterData()
      .getReporterContextBase(from_reporter)
      .declareClone(
          to_problem.getReporterData(ReporterData::WriteKey()), to_reporter, mode, _rti_transfer);

  // Hide variables (if requested in parameters) if name is associated with a reporter object
  hideVariableHelper(to_reporter, to_problem);
}

void
ReporterTransferInterface::declareClone(const ReporterName & rname,
                                        FEProblemBase & problem,
                                        const std::string & type,
                                        const ReporterMode & mode)
{
  ReporterData & rdata = problem.getReporterData(ReporterData::WriteKey());
  if (type == "bool")
    rdata.declareReporterValue<bool, ReporterGeneralContext<bool>>(rname, mode, _rti_transfer);
  else if (type == "integer")
    rdata.declareReporterValue<int, ReporterGeneralContext<int>>(rname, mode, _rti_transfer);
  else if (type == "real")
    rdata.declareReporterValue<Real, ReporterGeneralContext<Real>>(rname, mode, _rti_transfer);
  else if (type == "string")
    rdata.declareReporterValue<std::string, ReporterGeneralContext<std::string>>(
        rname, mode, _rti_transfer);
  else
    _rti_transfer.mooseError("Unknown reporter type, ", type, ".");

  // Hide variables (if requested in parameters) if name is associated with a reporter object
  hideVariableHelper(rname, problem);
}

void
ReporterTransferInterface::declareVectorClone(const ReporterName & from_reporter,
                                              const ReporterName & to_reporter,
                                              const FEProblemBase & from_problem,
                                              FEProblemBase & to_problem,
                                              const ReporterMode & mode)
{
  checkHasReporterValue(from_reporter, from_problem);
  from_problem.getReporterData()
      .getReporterContextBase(from_reporter)
      .declareVectorClone(
          to_problem.getReporterData(ReporterData::WriteKey()), to_reporter, mode, _rti_transfer);

  // Hide variables (if requested in parameters) if name is associated with a reporter object
  hideVariableHelper(to_reporter, to_problem);
}

void
ReporterTransferInterface::declareVectorClone(const ReporterName & rname,
                                              FEProblemBase & problem,
                                              const std::string & type,
                                              const ReporterMode & mode)
{
  ReporterData & rdata = problem.getReporterData(ReporterData::WriteKey());
  if (type == "bool")
    rdata.declareReporterValue<std::vector<bool>, ReporterVectorContext<bool>>(
        rname, mode, _rti_transfer);
  else if (type == "integer")
    rdata.declareReporterValue<std::vector<int>, ReporterVectorContext<int>>(
        rname, mode, _rti_transfer);
  else if (type == "real")
    rdata.declareReporterValue<std::vector<Real>, ReporterVectorContext<Real>>(
        rname, mode, _rti_transfer);
  else if (type == "string")
    rdata.declareReporterValue<std::vector<std::string>, ReporterVectorContext<std::string>>(
        rname, mode, _rti_transfer);
  else
    _rti_transfer.mooseError("Unknown reporter type, ", type, ".");

  // Hide variables (if requested in parameters) if name is associated with a reporter object
  hideVariableHelper(rname, problem);
}

void
ReporterTransferInterface::resizeReporter(const ReporterName & name,
                                          FEProblemBase & problem,
                                          dof_id_type n)
{
  checkHasReporterValue(name, problem);
  problem.getReporterData(ReporterData::WriteKey()).getReporterContextBase(name).resize(n);
}
void
ReporterTransferInterface::clearVectorReporter(const ReporterName & name, FEProblemBase & problem)
{
  checkHasReporterValue(name, problem);
  problem.getReporterData(ReporterData::WriteKey()).getReporterContextBase(name).clear();
}

void
ReporterTransferInterface::sumVectorReporter(const ReporterName & name, FEProblemBase & problem)
{
  checkHasReporterValue(name, problem);
  problem.getReporterData(ReporterData::WriteKey()).getReporterContextBase(name).vectorSum();
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

void
ReporterTransferInterface::checkHasReporterValue(const ReporterName & reporter,
                                                 const FEProblemBase & problem) const
{
  if (!problem.getReporterData().hasReporterValue(reporter))
    _rti_transfer.mooseError("Reporter with the name \"",
                             reporter,
                             "\" within app \"",
                             problem.getMooseApp().name(),
                             "\" was not found.");
}

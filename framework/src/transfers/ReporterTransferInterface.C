//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterTransferInterface.h"

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

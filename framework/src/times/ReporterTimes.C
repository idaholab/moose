//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterTimes.h"

registerMooseObject("MooseApp", ReporterTimes);

InputParameters
ReporterTimes::validParams()
{
  InputParameters params = Times::validParams();
  params.addRequiredParam<std::vector<ReporterName>>("reporters",
                                                     "Reporter(s) containing the times.");

  // User reporter broadcasting behavior
  params.set<bool>("auto_broadcast") = false;

  params.addClassDescription("Import times from one or more reporters, for example other Times");
  return params;
}

ReporterTimes::ReporterTimes(const InputParameters & parameters) : Times(parameters)
{
  // Attempt to obtain the positions. Will only succeed for other Times at this point
  initialize();
  // Broadcast if needed
  finalize();
}

void
ReporterTimes::initialize()
{
  clearTimes();

  const auto & positions_reporters = getParam<std::vector<ReporterName>>("reporters");

  for (const auto r_it : index_range(positions_reporters))
  {
    const auto & reporter_name = positions_reporters[r_it];

    const auto & reporter_data = _fe_problem.getReporterData();
    if (reporter_data.getReporterContextBase(reporter_name).getProducerModeEnum() ==
        REPORTER_MODE_DISTRIBUTED)
      mooseError("Distributed reporter not implemented yet");
    const auto & data = reporter_data.getReporterValue<std::vector<Real>>(reporter_name);

    for (const auto & d : data)
    {
      _times.push_back(d);
    }
  }
}

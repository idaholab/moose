//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterPositions.h"

registerMooseObject("MooseApp", ReporterPositions);

InputParameters
ReporterPositions::validParams()
{
  InputParameters params = Positions::validParams();
  params.addRequiredParam<std::vector<ReporterName>>("reporters",
                                                     "Reporter(s) containing the positions");
  params.addClassDescription(
      "Import positions from one or more reporters, for example other Positions");
  return params;
}

ReporterPositions::ReporterPositions(const InputParameters & parameters) : Positions(parameters)
{
  // TODO Check execute_on. I'm not sure how to retrieve execute_on for reporters.
}

void
ReporterPositions::initialize()
{
  std::vector<ReporterName> positions_reporters = getParam<std::vector<ReporterName>>("reporters");
  _positions_2d.resize(positions_reporters.size());

  for (unsigned int r_it = 0; r_it < positions_reporters.size(); r_it++)
  {
    const std::string reporter_name = positions_reporters[r_it];

    const auto & reporter_data = _fe_problem.getReporterData();
    if (reporter_data.getReporterContextBase(reporter_name).getProducerModeEnum() ==
        REPORTER_MODE_DISTRIBUTED)
      mooseError("Distributed reporter not implemented yet");
    const auto & data = reporter_data.getReporterValue<std::vector<Point>>(reporter_name);

    for (const auto & d : data)
    {
      _positions.push_back(d);
      _positions_2d[r_it].push_back(d);
    }
  }
}

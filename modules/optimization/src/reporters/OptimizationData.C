//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationData.h"

registerMooseObject("isopodApp", OptimizationData);

InputParameters
OptimizationData::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription(
      "Reporter to hold measurement and simulation data for optimization problems");
  return params;
}

OptimizationData::OptimizationData(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _measurement_xcoord(
        declareValueByName<std::vector<Real>>("measurement_xcoord", REPORTER_MODE_REPLICATED)),
    _measurement_ycoord(
        declareValueByName<std::vector<Real>>("measurement_ycoord", REPORTER_MODE_REPLICATED)),
    _measurement_zcoord(
        declareValueByName<std::vector<Real>>("measurement_zcoord", REPORTER_MODE_REPLICATED)),
    _measurement_values(
        declareValueByName<std::vector<Real>>("measurement_values", REPORTER_MODE_REPLICATED)),
    _simulation_values(
        declareValueByName<std::vector<Real>>("simulation_values", REPORTER_MODE_REPLICATED)),
    _misfit_values(declareValueByName<std::vector<Real>>("misfit_values", REPORTER_MODE_REPLICATED))
{
  if (isParamValid("measurement_points"))
  {
    std::vector<Point> measurement_points = getParam<std::vector<Point>>("measurement_points");
    for (auto & p : measurement_points)
    {
      _measurement_xcoord.push_back(p(0));
      _measurement_ycoord.push_back(p(1));
      _measurement_zcoord.push_back(p(2));
    }
  }

  if (isParamValid("measurement_values"))
    _measurement_values = getParam<std::vector<Real>>("measurement_values");
}

void
OptimizationData::execute()
{
}

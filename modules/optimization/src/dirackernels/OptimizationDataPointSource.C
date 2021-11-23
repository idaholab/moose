//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptimizationDataPointSource.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", OptimizationDataPointSource);

defineLegacyParams(OptimizationDataPointSource);

InputParameters
OptimizationDataPointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();

  params.addClassDescription("Apply misfit point loads from optimization_data.");

  params.addRequiredParam<ReporterName>("optimization_data", "fixme this is the tuple.");
  return params;
}

OptimizationDataPointSource::OptimizationDataPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    ReporterInterface(this),
    _optimization_data(getReporterValue<std::vector<std::tuple<Point, Real, Real, Real>>>(
        "optimization_data", REPORTER_MODE_REPLICATED))
{
}

void
OptimizationDataPointSource::addPoints()
{
  _point_to_index.clear();
  for (size_t i = 0; i < _optimization_data.size(); ++i)
  {
    Point pt(std::get<0>(_optimization_data[i]));
    _point_to_index[pt] = i;
    addPoint(pt, i);
  }
}

Real
OptimizationDataPointSource::computeQpResidual()
{
  //  This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp] * std::get<3>(_optimization_data[_point_to_index[_current_point]]);
}

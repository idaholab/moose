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
#include "OptimizationData.h"

registerMooseObject("MooseApp", OptimizationDataPointSource);

defineLegacyParams(OptimizationDataPointSource);

InputParameters
OptimizationDataPointSource::validParams()
{
  InputParameters params = DiracKernel::validParams();

  params.addClassDescription("Apply misfit point loads from optimization_data.");

  params.addRequiredParam<ReporterName>(
      "points", "Reporter containing points.  This uses the reporter syntax <reporter>/<name>.");
  params.addRequiredParam<ReporterName>(
      "values", "Reporter containing values.  This uses the reporter syntax <reporter>/<name>.");

  return params;
}

OptimizationDataPointSource::OptimizationDataPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    ReporterInterface(this),
    _points(getReporterValue<std::vector<Point>>("points", REPORTER_MODE_REPLICATED)),
    _values(getReporterValue<std::vector<Real>>("values", REPORTER_MODE_REPLICATED))
{
  std::cout << "****************************************OptimizationDataPointSource::"
               "OptimizationDataPointSource"
            << std::endl;
}

void
OptimizationDataPointSource::addPoints()
{
  if (_values.size() != _points.size())
  {
    std::string errMsg =
        "The value and points vectors are a different size.  \n"
        "There must be one value per point.  If the sizes are \n"
        "zero, the reporter or reporter may not have been initialized with data \n"
        "before the Dirac Kernel is called.  \n"
        "Try setting \"execute_on = timestep_begin\" in the reporter being read. \n"
        "values size = " +
        std::to_string(_values.size()) + ";  points size = " + std::to_string(_points.size());

    mooseError(errMsg);
  }
  // fixme, do I need _point_to_index, will the points get mixed up when it calls computeQPresidual
  _point_to_index.clear();
  for (size_t i = 0; i < _points.size(); ++i)
  {
    Point pt(_points[i]);
    _point_to_index[pt] = i;
    addPoint(pt, i);
  }
}

Real
OptimizationDataPointSource::computeQpResidual()
{
  //  This is negative because it's a forcing function that has been brought over to the left side
  return -_test[_i][_qp] * _values[_point_to_index[_current_point]];
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmootherChainControl.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", SmootherChainControl);

InputParameters
SmootherChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();

  params.addClassDescription("Computes a moving average of the input control with a user-specified "
                             "number of points to average.");

  params.addRequiredParam<std::string>("input", "Control data value to smooth.");
  params.addRequiredParam<unsigned int>("n_points",
                                        "The number of points to use in the moving average.");

  return params;
}

SmootherChainControl::SmootherChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    _input(getChainControlData<Real>("input")),
    _n_points(getParam<unsigned int>("n_points")),
    _output(declareChainControlData<Real>("value")),
    _values(declareRestartableData<std::vector<Real>>("values")),
    _previous_time(declareRestartableData<Real>("previous_time"))
{
  _previous_time = std::numeric_limits<Real>::max();
}

void
SmootherChainControl::execute()
{
  if (!MooseUtils::absoluteFuzzyEqual(_t, _previous_time))
    executeInner();

  _previous_time = _t;
}

void
SmootherChainControl::executeInner()
{
  if (_values.size() == _n_points)
    _values.erase(_values.begin());

  _values.push_back(_input);

  _output = std::accumulate(_values.begin(), _values.end(), 0.0) / _values.size();
}

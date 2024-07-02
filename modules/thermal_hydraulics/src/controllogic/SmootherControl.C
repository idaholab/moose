//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SmootherControl.h"

registerMooseObject("ThermalHydraulicsApp", SmootherControl);

InputParameters
SmootherControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<std::string>("input", "Control data value to smooth.");
  params.addRequiredParam<unsigned int>("n_points",
                                        "The number of points to use in the moving average.");
  params.addClassDescription("Computes a moving average value of the input control "
                             "with a user-specified number of points to average. "
                             "The output control value is named 'name:value', "
                             "where 'name' is the name of the control object.");
  return params;
}

SmootherControl::SmootherControl(const InputParameters & parameters)
  : THMControl(parameters),
    _input(getControlData<Real>("input")),
    _n_points(getParam<unsigned int>("n_points")),
    _output(declareComponentControlData<Real>("value")),
    _values(declareRestartableData<std::vector<Real>>("values"))
{
}

void
SmootherControl::execute()
{
  if (_fe_problem.getCurrentExecuteOnFlag() == EXEC_TIMESTEP_BEGIN)
  {
    if (_values.size() == _n_points)
      _values.erase(_values.begin());

    _values.push_back(_input);

    _output = std::accumulate(_values.begin(), _values.end(), 0.0) / _values.size();
  }
}

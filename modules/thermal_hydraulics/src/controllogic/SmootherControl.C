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
  params.addRequiredParam<std::string>("input", "The name of the dataset.");
  params.addRequiredParam<unsigned int>("n_points",
                                        "The maximum number of data points to be "
                                        "used in the moving average calculation.");
  params.addClassDescription("Declares a control data named 'value' and uses a moving average "
                             "on the 'value' control data to smooth it.");
  return params;
}

SmootherControl::SmootherControl(const InputParameters & parameters)
  : THMControl(parameters),
    _input(getControlData<Real>("input")),
    _n_points(getParam<unsigned int>("n_points")),
    _output(declareComponentControlData<Real>("value"))
{
}

void
SmootherControl::execute()
{
  if (_values.size() == _n_points)
    _values.erase(_values.begin());

  _values.push_back(_input);

  _output = std::accumulate(_values.begin(), _values.end(), 0.0) / _values.size();
}

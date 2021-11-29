//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoarsenedPiecewiseLinear.h"
#include "PointReduction.h"

registerMooseObject("MooseApp", CoarsenedPiecewiseLinear);

InputParameters
CoarsenedPiecewiseLinear::validParams()
{
  InputParameters params = PiecewiseLinearBase::validParams();
  params.addClassDescription("Perform a point reduction of the tabulated data upon initialization, "
                             "then evaluate using a linear interpolation.");
  params.addRequiredParam<Real>(
      "epsilon",
      "Significant distance in the function below which points are considered removable noise");
  params.addParam<Real>("y_scale",
                        1.0,
                        "Scaling factor to apply to the function nodes for the purpose of "
                        "computing distances in the Douglas-Peucker point reduction algorithm. "
                        "This permits shifting the weight between x and y-direction distances.");
  params.addParam<Real>("x_scale",
                        1.0,
                        "Scaling factor to apply to the function nodes for the purpose of "
                        "computing distances in the Douglas-Peucker point reduction algorithm. "
                        "This permits shifting the weight between x and y-direction distances.");
  return params;
}

CoarsenedPiecewiseLinear::CoarsenedPiecewiseLinear(const InputParameters & parameters)
  : PiecewiseLinearBase(parameters)
{
  const Real x_scale = getParam<Real>("x_scale");
  const Real y_scale = getParam<Real>("y_scale");
  const Real epsilon = getParam<Real>("epsilon");

  // create vector of pairs
  PointReduction::FunctionNodeList list;
  list.reserve(_raw_x.size());
  for (MooseIndex(_raw_x) i = 0; i < _raw_x.size(); ++i)
    list.emplace_back(_raw_x[i] * x_scale, _raw_y[i] * y_scale);

  // point reduction
  _console << "Reduced size for function '" << name() << "' from " << list.size();
  list = PointReduction::douglasPeucker(list, epsilon);
  _console << " to " << list.size() << " points." << std::endl;

  // unpack vector of pairs
  _raw_x.resize(list.size());
  _raw_y.resize(list.size());
  for (MooseIndex(list) i = 0; i < list.size(); ++i)
  {
    _raw_x[i] = list[i].first / x_scale;
    _raw_y[i] = list[i].second / y_scale;
  }

  buildInterpolation();
}

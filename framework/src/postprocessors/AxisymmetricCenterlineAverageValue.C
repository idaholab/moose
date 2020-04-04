//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AxisymmetricCenterlineAverageValue.h"
#include "libmesh/quadrature.h"

registerMooseObject("MooseApp", AxisymmetricCenterlineAverageValue);

InputParameters
AxisymmetricCenterlineAverageValue::validParams()
{
  InputParameters params = SideAverageValue::validParams();
  params.addClassDescription("Computes the average value of a variable on a "
                             "sideset located along the centerline of an "
                             "axisymmetric model.");

  return params;
}

AxisymmetricCenterlineAverageValue::AxisymmetricCenterlineAverageValue(
    const InputParameters & parameters)
  : SideAverageValue(parameters), _volume(0)
{
}

// NOTE: We do not account for the coordinate system transformation here
// (using _coord) because we want to average a variable over the centerline
// of a cylinder. If we weight by _coord, we get 0 / 0.
Real
AxisymmetricCenterlineAverageValue::volume()
{
  return _current_side_elem->volume();
}

Real
AxisymmetricCenterlineAverageValue::computeIntegral()
{
  Real sum = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    sum += _JxW[_qp] * computeQpIntegral();
  return sum;
}

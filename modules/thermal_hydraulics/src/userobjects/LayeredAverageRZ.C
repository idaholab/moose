//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredAverageRZ.h"

registerMooseObject("ThermalHydraulicsApp", LayeredAverageRZ);

InputParameters
LayeredAverageRZ::validParams()
{
  InputParameters params = LayeredAverage::validParams();
  params += RZSymmetry::validParams();
  params.addRequiredParam<Real>("length",
                                "The length of the block in the direction given by 'axis_dir'.");
  return params;
}

LayeredAverageRZ::LayeredAverageRZ(const InputParameters & parameters)
  : LayeredAverage(parameters), RZSymmetry(this, parameters)
{
  _direction_min = getParam<Point>("axis_point")(_direction);
  _direction_max = _direction_min + getParam<Real>("length");
}

void
LayeredAverageRZ::execute()
{
  LayeredIntegral::execute();

  Real current_elem_volume = 0.;
  for (unsigned int qp = 0; qp < _qrule->n_points(); qp++)
  {
    const Real circumference = computeCircumference(_q_point[qp]);
    current_elem_volume += _JxW[qp] * circumference;
  }

  unsigned int layer = getLayer(_current_elem->vertex_average());
  _layer_volumes[layer] += current_elem_volume;
}

Real
LayeredAverageRZ::computeIntegral()
{
  Real sum = 0;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    const Real circumference = computeCircumference(_q_point[_qp]);
    sum += _JxW[_qp] * circumference * computeQpIntegral();
  }
  return sum;
}

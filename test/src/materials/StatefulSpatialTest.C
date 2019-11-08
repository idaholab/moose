//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StatefulSpatialTest.h"

registerMooseObject("MooseTestApp", StatefulSpatialTest);

InputParameters
StatefulSpatialTest::validParams()
{
  InputParameters params = Material::validParams();
  return params;
}

StatefulSpatialTest::StatefulSpatialTest(const InputParameters & parameters)
  : Material(parameters),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_old(getMaterialPropertyOld<Real>("thermal_conductivity"))
{
}

void
StatefulSpatialTest::computeQpProperties()
{
  _thermal_conductivity[_qp] = _thermal_conductivity_old[_qp] + 1.;
}

void
StatefulSpatialTest::initQpStatefulProperties()
{
  _thermal_conductivity[_qp] = _t_step + (_q_point[_qp](0) * _q_point[_qp](1));
}

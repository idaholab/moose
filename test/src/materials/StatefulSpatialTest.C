/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "StatefulSpatialTest.h"

template <>
InputParameters
validParams<StatefulSpatialTest>()
{
  InputParameters params = validParams<Material>();
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

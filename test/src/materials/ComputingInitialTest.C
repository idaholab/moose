//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputingInitialTest.h"

registerMooseObject("MooseTestApp", ComputingInitialTest);

InputParameters
ComputingInitialTest::validParams()
{
  return Material::validParams();
}

ComputingInitialTest::ComputingInitialTest(const InputParameters & parameters)
  : Material(parameters),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_old(getMaterialPropertyOld<Real>("thermal_conductivity"))
{
}

void
ComputingInitialTest::initQpStatefulProperties()
{
  _thermal_conductivity[_qp] = 0.0;
}

void
ComputingInitialTest::computeQpProperties()
{
  if (_subproblem.computingInitialResidual())
    _thermal_conductivity[_qp] = _thermal_conductivity_old[_qp] + 1.0;
}

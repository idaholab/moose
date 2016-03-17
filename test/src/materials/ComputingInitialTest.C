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
#include "ComputingInitialTest.h"

template<>
InputParameters validParams<ComputingInitialTest>()
{
  InputParameters params = validParams<Material>();
  return params;
}

ComputingInitialTest::ComputingInitialTest(const InputParameters & parameters) :
    Material(parameters),
    _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
    _thermal_conductivity_old(declarePropertyOld<Real>("thermal_conductivity"))
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

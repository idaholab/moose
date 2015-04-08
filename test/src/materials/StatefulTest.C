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
#include "StatefulTest.h"

template<>
InputParameters validParams<StatefulTest>()
{
  InputParameters params = validParams<Material>();
  return params;
}

StatefulTest::StatefulTest(const std::string & name, InputParameters parameters)
  :Material(name, parameters),
   _thermal_conductivity(declareProperty<Real>("thermal_conductivity")),
   _thermal_conductivity_old(declarePropertyOld<Real>("thermal_conductivity")),
   _thermal_conductivity_older(declarePropertyOlder<Real>("thermal_conductivity"))
{}

void
StatefulTest::initQpStatefulProperties()
{
  _thermal_conductivity[_qp] = 1.0;
}

void
StatefulTest::computeQpProperties()
{
  // Really Expensive Fibonacci sequence generator!
  _thermal_conductivity[_qp] = _thermal_conductivity_old[_qp] + _thermal_conductivity_older[_qp];
}

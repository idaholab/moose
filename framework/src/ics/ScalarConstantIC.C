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

#include "ScalarConstantIC.h"

template<>
InputParameters validParams<ScalarConstantIC>()
{
  InputParameters params = validParams<ScalarInitialCondition>();
  params.set<Real>("value") = 0.0;
  return params;
}

ScalarConstantIC::ScalarConstantIC(const std::string & name, InputParameters parameters) :
    ScalarInitialCondition(name, parameters),
    _value(getParam<Real>("value"))
{
}

Real
ScalarConstantIC::value()
{
  return _value;
}


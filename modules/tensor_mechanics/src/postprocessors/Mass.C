//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

//  This post processor returns the mass value of an element.  It is used
//  to check that mass is conserved (per the evolving density calculation)
//  when volume changes occur.
//
#include "Mass.h"

template <>
InputParameters
validParams<Mass>()
{
  InputParameters params = validParams<ElementIntegralVariablePostprocessor>();
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

Mass::Mass(const InputParameters & parameters)
  : ElementIntegralVariablePostprocessor(parameters), _density(getMaterialProperty<Real>("density"))
{
}

Real
Mass::computeQpIntegral()
{
  return _density[_qp];
}

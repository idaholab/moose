/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

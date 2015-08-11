/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Gravity.h"

template<>
InputParameters validParams<Gravity>()
{
  InputParameters params = validParams<BodyForce>();
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

Gravity::Gravity(const InputParameters & parameters) :
  BodyForce(parameters),
  _density(getMaterialProperty<Real>("density"))
{
}

Real
Gravity::computeQpResidual()
{
  return _density[_qp] * BodyForce::computeQpResidual();
}

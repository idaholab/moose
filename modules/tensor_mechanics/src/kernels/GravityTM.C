/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GravityTM.h"

template<>
InputParameters validParams<GravityTM>()
{
  InputParameters params = validParams<BodyForce>();
  params.addClassDescription("Apply gravity. Value is in units of acceleration.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

GravityTM::GravityTM(const std::string & name, InputParameters parameters) :
  BodyForce(name, parameters),
  _density(getMaterialProperty<Real>("density"))
{
}

Real
GravityTM::computeQpResidual()
{
  return _density[_qp] * BodyForce::computeQpResidual();
}


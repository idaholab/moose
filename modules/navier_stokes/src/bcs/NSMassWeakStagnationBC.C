/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSMassWeakStagnationBC.h"

template <>
InputParameters
validParams<NSMassWeakStagnationBC>()
{
  InputParameters params = validParams<NSWeakStagnationBaseBC>();
  params.addClassDescription("The inviscid energy BC term with specified normal flow.");
  return params;
}

NSMassWeakStagnationBC::NSMassWeakStagnationBC(const InputParameters & parameters)
  : NSWeakStagnationBaseBC(parameters)
{
}

Real
NSMassWeakStagnationBC::computeQpResidual()
{
  // rho_s * |u| * (s.n) * phi_i
  return rhoStatic() * std::sqrt(this->velmag2()) * this->sdotn() * _test[_i][_qp];
}

Real
NSMassWeakStagnationBC::computeQpJacobian()
{
  // TODO
  return 0.0;
}

Real
NSMassWeakStagnationBC::computeQpOffDiagJacobian(unsigned /*jvar*/)
{
  // TODO
  return 0.0;
}

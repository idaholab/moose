//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NSMassWeakStagnationBC.h"

registerMooseObject("NavierStokesApp", NSMassWeakStagnationBC);

InputParameters
NSMassWeakStagnationBC::validParams()
{
  InputParameters params = NSWeakStagnationBaseBC::validParams();
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

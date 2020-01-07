//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBC.h"

class Function;

/**
 * Implements a simple BC for DG
 *
 * BC derived from convection problem that can handle:
 * velocity * n_e * u_up * [v]
 *
 * [a] = [ a_1 - a_2 ]
 * u_up = u|E_e_1 if velocity.n_e >= 0
 *        u|E_e_2 if velocity.n_e < 0
 *        with n_e pointing from E_e_1 to E_e_2
 *
 */

class DGFunctionConvectionDirichletBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  DGFunctionConvectionDirichletBC(const InputParameters & parameters);

  virtual ~DGFunctionConvectionDirichletBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  const Function & _func;

  RealVectorValue _velocity;

  Real _x;
  Real _y;
  Real _z;
};

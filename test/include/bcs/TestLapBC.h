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

/**
 * This BC enforces: grad(u) \cdot n = (1/2)*Lap(u) weakly.  This is
 * consistent with the exact solution, for which we know that:
 * grad(u).n = 2 on the right-hand boundary, and Lap(u) = 4
 * everywhere.  This is just an artificial way to introduce the
 * Laplacian into the BC.  There is a minus sign since this term is
 * moved to the LHS of the equation when the residual is formed.
 */
class TestLapBC : public IntegratedBC
{
public:
  static InputParameters validParams();

  TestLapBC(const InputParameters & parameters);
  virtual ~TestLapBC() {}

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const VariableSecond & _second_u;
  const VariablePhiSecond & _second_phi;
};

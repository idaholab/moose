//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "BoundaryFlux3EqnGhostBase.h"

/**
 * Computes flux for wall boundary conditions for the 1-D, 1-phase, variable-area Euler equations
 */
class BoundaryFlux3EqnGhostWall : public BoundaryFlux3EqnGhostBase
{
public:
  BoundaryFlux3EqnGhostWall(const InputParameters & parameters);

protected:
  virtual std::vector<Real> getGhostCellSolution(const std::vector<Real> & U1) const override;
  virtual DenseMatrix<Real>
  getGhostCellSolutionJacobian(const std::vector<Real> & U1) const override;

public:
  static InputParameters validParams();
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DirectDirichletBCBase.h"

/**
 * Boundary condition of a Direct Dirichlet type
 * For use with direct central difference time integrator
 *
 * Sets the value in the node
 */
class DirectDirichletBC : public DirectDirichletBCBase
{
public:
  static InputParameters validParams();

  DirectDirichletBC(const InputParameters & parameters);

protected:
  virtual Real computeQpValue() override;

  /// The value for this BC
  const Real & _value;

  /// Vector of 1's to help with creating the lumped mass matrix
  NumericVector<Real> * _ones;
};

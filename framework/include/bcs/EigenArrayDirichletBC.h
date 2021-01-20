//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ArrayNodalBC.h"

/**
 * Boundary condition of a Dirichlet type for the eigen side
 *
 * Sets the values to be zero
 */
class EigenArrayDirichletBC : public ArrayNodalBC
{
public:
  static InputParameters validParams();

  EigenArrayDirichletBC(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(RealEigenVector & residual) override;

  virtual RealEigenVector computeQpJacobian() override;
  virtual RealEigenMatrix computeQpOffDiagJacobian(MooseVariableFEBase & jvar) override;
};

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernel.h"

/**
 *  VectorInterfaceKernel that enforces the equivalence of the parallel vector
 *  field components on either side of a boundary
 */
class ParallelElectricFieldInterface : public VectorInterfaceKernel
{
public:
  static InputParameters validParams();

  ParallelElectricFieldInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  /// Parallel component of the solution vector field on the primary side of the boundary
  RealVectorValue _u_parallel;

  /// Parallel component of the solution vector field on the secondary side of the boundary
  RealVectorValue _secondary_parallel;

  /// Parallel component of the test function on the primary side of the boundary
  RealVectorValue _phi_u_parallel;

  /// Parallel component of the test function on the secondary side of the boundary
  RealVectorValue _phi_secondary_parallel;
};

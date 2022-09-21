//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGKernel.h"

/**
 * The HFEMTrialJump class computes an L2 product (lambda*, [u])_Gamma,
 * where Gamma is the set of internal sides, lambda* is a test
 * function corresponding to variation in a variable lambda (in
 * typical use cases, a SIDE_DISCONTINUOUS FEType like SIDE_HIERARCIC)
 * which is uniquely defined on those sides (although not necessarily
 * their boundaries), and [u] is the jump across a side of a
 * DISCONTINUOUS variable u (e.g. a MONOMIAL type)
 *
 * In an HFEM formulation (and probably in most mixed formulations
 * that would use this kernel) this should be paired with an
 * HFEMTestJump kernel.
 */
class HFEMTrialJump : public DGKernel
{
public:
  static InputParameters validParams();

  HFEMTrialJump(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType) override { return 0; }
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  /// Variable whose jump to test against variations in lambda
  const MooseVariable & _interior_var;

  /// Current interior solution at the current side quadrature point
  const VariableValue & _interior;

  /// neighbor's interior solution at the current side quadrature point
  const VariableValue & _interior_neighbor;

  /// interior shape function at the current side quadrature point
  const VariablePhiValue & _phi_interior;

  /// neighbor's shape function at the current side quadrature point
  const VariablePhiValue & _phi_interior_neighbor;

  /// interior test function at the current side quadrature point
  const VariablePhiValue & _test_interior;

  /// Variable id for interior variable
  const unsigned int _interior_id;
};

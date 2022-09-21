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
 * The HFEMTestJump class computes an L2 product (lambda, [u*])_Gamma,
 * where Gamma is the set of internal sides, lambda is a variable
 * (generally a SIDE_DISCONTINUOUS FEType like SIDE_HIERARCIC) which
 * is uniquely defined on those sides (although not necessarily their
 * boundaries), and [u*] is the jump across a side of a test function
 * u* corresponding to variation in a DISCONTINUOUS variable u (e.g.
 * a MONOMIAL type)
 *
 * In an HFEM formulation (and probably in most mixed formulations
 * that would use this kernel) this should be paired with an
 * HFEMTestJump kernel.
 */
class HFEMTestJump : public DGKernel
{
public:
  static InputParameters validParams();

  HFEMTestJump(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType) override { return 0; }
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  /// Variable for the lagrange multiplier lambda
  const MooseVariable & _lambda_var;

  /// Current lambda solution at the current side quadrature point
  const VariableValue & _lambda;

  /// lambda shape function at the current side quadrature point
  const VariablePhiValue & _phi_lambda;

  /// lambda test function at the current side quadrature point
  const VariablePhiValue & _test_lambda;

  /// Variable id for lambda
  const unsigned int _lambda_id;
};

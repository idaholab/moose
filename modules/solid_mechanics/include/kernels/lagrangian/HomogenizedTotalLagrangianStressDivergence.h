//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TotalLagrangianStressDivergence.h"

#include "HomogenizationConstraint.h"
#include "MooseVariableScalar.h"
#include "Assembly.h"

/// Total Lagrangian formulation with cross-jacobian homogenization terms
///
///  The total Lagrangian formulation can interact with the homogenization
///  system defined by the HomogenizationConstraintScalarKernel and
///  HomogenizationConstraint user object by providing the
///  correct off-diagonal Jacobian entries.
///
class HomogenizedTotalLagrangianStressDivergence : public TotalLagrangianStressDivergence
{
public:
  static InputParameters validParams();
  HomogenizedTotalLagrangianStressDivergence(const InputParameters & parameters);

protected:
  /// Homogenization constraint diagonal term
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

  /// The scalar variable used to enforce the homogenization constraints
  const unsigned int _macro_gradient_num;

  // Which indices are constrained and what types of constraints
  const HomogenizationConstraint & _constraint;

  /// The constraint map
  const Homogenization::ConstraintMap & _cmap;

  /// Derivatives of ivar with respect to jvar
  DenseMatrix<Number> _ken;
  /// Derivatives of jvar with respect to ivar
  DenseMatrix<Number> _kne;
};

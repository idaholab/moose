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

#include "HomogenizationConstraintIntegral.h" // Index constants
#include "MooseVariableScalar.h"
#include "Assembly.h"

/// Total Lagrangian formulation with cross-jacobian homogenization terms
///
///  The total Lagrangian formulation can interact with the homogenization
///  system defined by the HomogenizationConstraintScalarKernel and
///  HomogenizationConstraintIntegral user object by providing the
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

private:
  /// Calculate the displacement-scalar part of the off-diagonal Jacobian
  Real computeBaseJacobian();

  /// Calculate the scalar-base part of the off-diagonal Jacobian
  //    Properly this would belong in the ScalarKernel, but as it varies
  //    by element it's best to put it here
  Real computeConstraintJacobian();

  /// Calculate the material part of the base disp-scalar jacobian
  Real materialBaseJacobian();

  /// Small deformation scalar-displacement component for strain constraints
  Real sdConstraintJacobianStrain();

  /// Large deformation scalar-displacement component for strain constraints
  Real ldConstraintJacobianStrain();

  /// Material scalar-displacement component for stress constraints
  Real materialConstraintJacobianStress();

protected:
  /// The scalar variable used to enforce the homogenization constraints
  const unsigned int _macro_gradient_num;
  /// Order of the homogenization variable, used in several places
  const unsigned int _mg_order;

  // Which indices are constrained and what types of constraints
  const HomogenizationConstants::index_list _indices;
  std::vector<HomogenizationConstants::ConstraintType> _ctypes;

  /// Used internally to iterate over each scalar component
  unsigned int _h;
};

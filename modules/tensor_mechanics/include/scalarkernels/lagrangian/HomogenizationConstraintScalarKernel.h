//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernel.h"

#include "HomogenizationConstraint.h"

/// Enforces a cell-average constraint
///
///  This kernel enforces a cell-average constraint of the type
///  \int_{V}\left(X_{ij}-\hat{X}_{ij}\right)dV, where $X$
///  is some stress or strain quantity by changing the value
///  of a scalar variable, imposed by CalculateStrainLagrangianKernel
///  as an "extra" additional strain or deformation gradient contribution
///
///  It only works with the TotalLagrangianStressDivergence kernel
///  but works for both large and small deformations
///
///  It relies on the HomogenizationConstraint to actually
///  calculate the residual and jacobian, as they are volume integrals over
///  the domain.
///
///  Also: the scalar-displacement off-diagonal entry, which would be
///  included in this object ideally, instead is implemented in
///  the TotalLagrangianStressDivergence because it varies element by
///  element (and that kernel is already visiting each quadrature point)
///
class HomogenizationConstraintScalarKernel : public ScalarKernel
{
public:
  static InputParameters validParams();

  HomogenizationConstraintScalarKernel(const InputParameters & parameters);

  virtual void reinit();
  /// Copies the residual from the user object
  virtual void computeResidual();
  /// Copies the on-diagonal Jacobian from the user object
  virtual void computeJacobian();

protected:
  /// The user object that does the actual volume integral
  const HomogenizationConstraint & _constraint;

  /// The actual tensor residual, from the user object
  const RankTwoTensor & _residual;
  /// The actual tensor jacobian, from the user object
  const RankFourTensor & _jacobian;
  /// The constraint map
  const Homogenization::ConstraintMap & _cmap;
};

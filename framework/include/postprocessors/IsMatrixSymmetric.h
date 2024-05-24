//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

namespace libMesh
{
template <typename>
class PetscMatrix;
}

/**
 * Checks whether the nonlinear system matrix is symmetric
 */
class IsMatrixSymmetric : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  IsMatrixSymmetric(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override;

  virtual Real getValue() const override;

protected:
  /// Tolerance for the comparison between coefficients and transpose counterparts
  const Real _symm_tol;
  /// Transpose of the system matrix
  std::unique_ptr<SparseMatrix<Number>> _mat_transpose;
  /// Whether the matrix is symmetric
  bool _equiv = true;
};

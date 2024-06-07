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

  virtual ~IsMatrixSymmetric();

protected:
  /// Tolerance for the comparison between coefficients and transpose counterparts
  const Real _symm_tol;
  /// a binary file for loading the matrix (optional)
  const FileName & _mat_file;
  /// The matrix we are testing for symmetry
  const SparseMatrix<Number> * _mat;
  /// Transpose of the matrix we are testing
  std::unique_ptr<SparseMatrix<Number>> _mat_transpose;
  /// Storage for the libMesh wrapper of the PETSc matrix, if it is provided
  std::unique_ptr<SparseMatrix<Number>> _wrapped_petsc_mat;

  /// PETSc mat
  Mat _binary_mat = nullptr;
  /// PETSc viewer
  PetscViewer _binary_matviewer = nullptr;

  /// Whether the matrix is symmetric
  bool _equiv = true;
};

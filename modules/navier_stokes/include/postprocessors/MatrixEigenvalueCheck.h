//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * Returns the number of zero eigenvalues in a PETSc matrix
 */
class MatrixEigenvalueCheck : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  MatrixEigenvalueCheck(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() const override;

protected:
  const Real _zero_tol;
  const std::string & _mat_name;
  const bool _print;
  Mat _petsc_mat = nullptr;
  Real _num_zero_eigenvalues = 0;
};

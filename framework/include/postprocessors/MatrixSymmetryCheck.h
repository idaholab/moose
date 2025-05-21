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

/**
 * Checks whether the nonlinear system matrix is symmetric
 */
class MatrixSymmetryCheck : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  MatrixSymmetryCheck(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual Real getValue() const override;

protected:
  /// Whether the matrix we are checking for symmetry is from a file
  const bool _mat_from_file;
  /// The matrix from file name. Empty string if \p _mat_from_file is \p false
  const std::string _mat_file_name;
  /// Tolerance for the comparison between coefficients and transpose counterparts
  const Real _symm_tol;
  /// A binary file may contain multiple writes of a matrix. This member can be used to load a
  /// particular matrix from the binary file
  const unsigned int _mat_number_to_load;
  ///  Whether the matrix is symmetric
  bool _equiv;
};

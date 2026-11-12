//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "Function.h"

class MFEMProblem;

/** Declares a named MFEM matrix whose entries borrow scalar coefficients. */
class MFEMMatrixCoefficientFunction : public Function
{
public:
  static InputParameters validParams();
  MFEMMatrixCoefficientFunction(const InputParameters & parameters);
  virtual ~MFEMMatrixCoefficientFunction() = default;
  void initialSetup() override;

protected:
  /// Scalar names or literals, stored in row-major order.
  const std::vector<std::vector<MFEMScalarCoefficientName>> & _entries;
  /// Problem owning both the matrix and its borrowed scalar coefficients.
  MFEMProblem & _mfem_problem;

private:
  /// Validate the parsed `matrix` parameter. It must be non-empty & square.
  /// Raise a `paramError` if the matrix is not valid.
  void raise_on_invalid_matrix(const std::vector<std::vector<MFEMScalarCoefficientName>> & mat);
};

#endif

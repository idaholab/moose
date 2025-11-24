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

#include "MFEMAuxKernel.h"

/**
 * Project s(x) * (U . V) onto a scalar MFEM auxvariable.
 *
 * Notes:
 *  - Currently supports only interior DOFs (no shared/constrained DOFs).
 *  - The target variable's FE Space must be L2.
 */
class MFEMDotProductAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMDotProductAux(const InputParameters & parameters);
  ~MFEMDotProductAux() override = default;

  void execute() override;

protected:
  /// Names of vector sources
  const VariableName _u_var_name;
  const VariableName _v_var_name;

  /// References to the vector ParGridFunctions
  const mfem::ParGridFunction & _u_var;
  const mfem::ParGridFunction & _v_var;
  const mfem::real_t _scale_factor;

  /// Coefficient wrappers
  mfem::VectorGridFunctionCoefficient _u_coef;
  mfem::VectorGridFunctionCoefficient _v_coef;
  mfem::InnerProductCoefficient _dot_uv;
  mfem::ConstantCoefficient _scale_c;

  ///Final coefficient that applies the scale factor to the inner product
  mfem::ProductCoefficient _final_coef;
};

#endif // MOOSE_MFEM_ENABLED

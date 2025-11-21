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

#include "MFEMComplexAuxKernel.h"
#include "mfem.hpp"

/**
 * Project s(x) * (U x V) onto a vector MFEM auxvariable.
 *
 * Notes:
 *   - Currently supports only interior DOFs (no shared/constrained DOFs).
 *   - Enforces 3D: mesh dimension and all involved vdim must be 3.
 */
class MFEMComplexCrossProductAux : public MFEMComplexAuxKernel
{
public:
  static InputParameters validParams();

  MFEMComplexCrossProductAux(const InputParameters & parameters);
  ~MFEMComplexCrossProductAux() override = default;

  void execute() override;

protected:
  /// Names of vector sources
  const VariableName _u_var_name;
  const VariableName _v_var_name;

  /// References to the vector ParGridFunctions
  const mfem::ParComplexGridFunction & _u_var;
  const mfem::ParComplexGridFunction & _v_var;
  /// Real part of the scaling factor applied on the resulting field
  const mfem::real_t _scale_factor_real;
  /// Imaginary part of the scaling factor applied on the resulting field
  const mfem::real_t _scale_factor_imag;

  /// Coefficient wrappers
  mfem::VectorGridFunctionCoefficient _u_coef_real;
  mfem::VectorGridFunctionCoefficient _u_coef_imag;
  mfem::VectorGridFunctionCoefficient _v_coef_real;
  mfem::VectorGridFunctionCoefficient _v_coef_imag;

  mfem::VectorCrossProductCoefficient _cross_ur_vr;
  mfem::VectorCrossProductCoefficient _cross_ur_vi;
  mfem::VectorCrossProductCoefficient _cross_ui_vr;
  mfem::VectorCrossProductCoefficient _cross_ui_vi;

  ///Final coefficient that sums the crossproduct terms
  mfem::VectorSumCoefficient _final_coef_real;
  mfem::VectorSumCoefficient _final_coef_imag;
};

#endif // MOOSE_MFEM_ENABLED

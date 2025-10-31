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
#include "mfem.hpp"

/**
 * Project s(x) * (U x V) onto a vector MFEM auxvariable.
 *
 * Parameters:
 *   - variable (AuxVariableName, required): AuxVariable name (inherited)
 *   - u (VariableName, required): vector MFEM variable U (vdim = 3)
 *   - v (VariableName, required): vector MFEM variable V (vdim = 3)
 *   - scale_factor (mfem::real_t, default=1.0): constant multiplier s(x)=scale_factor
 *
 * Notes:
 *   - Default L2_FECollection with map_type = VALUE and optional map_type = INTEGRAL .
 *   - Currently supports only interior DOFs (no shared/constrained DOFs).
 *   - Enforces 3D: mesh dimension and all involved vdim must be 3.
 */
class MFEMCrossProductAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMCrossProductAux(const InputParameters & parameters);
  ~MFEMCrossProductAux() override = default;

  void execute() override;

protected:
  // Names of vector sources
  const VariableName _u_var_name;
  const VariableName _v_var_name;

  // References to the vector ParGridFunctions
  const mfem::ParGridFunction & _u_var;
  const mfem::ParGridFunction & _v_var;
  const mfem::real_t _scale_factor;

  mfem::VectorGridFunctionCoefficient _u_coef;
  mfem::VectorGridFunctionCoefficient _v_coef;
  mfem::VectorCrossProductCoefficient _cross_uv;
  mfem::ConstantCoefficient _scale_c;
  mfem::ScalarVectorProductCoefficient _final_vec;

  // Constant multiplier
  // const mfem::real_t _scale_factor;
};

#endif // MOOSE_MFEM_ENABLED

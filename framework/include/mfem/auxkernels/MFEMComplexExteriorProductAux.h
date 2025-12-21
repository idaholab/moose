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

/**
 * Project \f$ s \vec u \wedge \vec v* \f$ onto a complex vector MFEM auxvariable.
 *
 * Notes:
 *  - Enforces 3D: all involved vdim must be 3.
 *  - The target variable's FE Space must be L2.
 *  - Currently supports only interior DOFs (no shared/constrained DOFs).
 */
class MFEMComplexExteriorProductAux : public MFEMComplexAuxKernel
{
public:
  static InputParameters validParams();

  MFEMComplexExteriorProductAux(const InputParameters & parameters);
  ~MFEMComplexExteriorProductAux() override = default;

  void execute() override;

protected:
  /// Scaling factor applied on the resulting field
  const std::complex<mfem::real_t> _scale_factor;

  /// Coefficient wrappers
  mfem::VectorCoefficient & _u_coef_real;
  mfem::VectorCoefficient & _u_coef_imag;
  mfem::VectorCoefficient & _v_coef_real;
  mfem::VectorCoefficient & _v_coef_imag;

  mfem::VectorCrossProductCoefficient _cross_ur_vr;
  mfem::VectorCrossProductCoefficient _cross_ur_vi;
  mfem::VectorCrossProductCoefficient _cross_ui_vr;
  mfem::VectorCrossProductCoefficient _cross_ui_vi;

  ///Final coefficient that sums the crossproduct terms
  mfem::VectorSumCoefficient _final_coef_real;
  mfem::VectorSumCoefficient _final_coef_imag;
};

#endif // MOOSE_MFEM_ENABLED

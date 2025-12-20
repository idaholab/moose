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
 * Project \f$ s \vec u \cdot \vec v* \f$ onto a complex scalar MFEM auxvariable.
 *
 * Notes:
 *  - The target variable's FE Space must be L2.
 *  - Currently supports only interior DOFs (no shared/constrained DOFs).
 */
class MFEMComplexInnerProductAux : public MFEMComplexAuxKernel
{
public:
  static InputParameters validParams();

  MFEMComplexInnerProductAux(const InputParameters & parameters);
  ~MFEMComplexInnerProductAux() override = default;

  void execute() override;

protected:
  /// Scaling factor applied on the resulting field
  const std::complex<mfem::real_t> _scale_factor;

  /// Coefficient wrappers
  mfem::VectorCoefficient & _u_coef_real;
  mfem::VectorCoefficient & _u_coef_imag;
  mfem::VectorCoefficient & _v_coef_real;
  mfem::VectorCoefficient & _v_coef_imag;

  mfem::InnerProductCoefficient _dot_ur_vr;
  mfem::InnerProductCoefficient _dot_ur_vi;
  mfem::InnerProductCoefficient _dot_ui_vr;
  mfem::InnerProductCoefficient _dot_ui_vi;

  ///Final coefficient that sums the inner product terms
  mfem::SumCoefficient _final_coef_real;
  mfem::SumCoefficient _final_coef_imag;
};

#endif // MOOSE_MFEM_ENABLED

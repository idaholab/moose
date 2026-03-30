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

#include "MFEMPostprocessor.h"
#include "MFEMBlockRestrictable.h"

/**
 * Computes the L2 norm of a real or complex scalar MFEM variable over the domain (or a set of
 * specified subdomains) and writes the normalized variable to an auxiliary gridfunction.
 *
 * The normalization constant is the L2 norm of the input variable:
 *   c = sqrt( integral_Omega u^2 dOmega )           (real)
 *   c = sqrt( integral_Omega |u|^2 dOmega )         (complex, |u|^2 = u_r^2 + u_i^2)
 *
 * The result written to the auxiliary variable is u / c, which satisfies ||u_aux||_2 = 1.
 * The postprocessor value returned by getValue() is the normalization constant c.
 * The input variable is left unchanged.
 */
class MFEMSquareMagnitudeIntegralNormalizationPostprocessor : public MFEMPostprocessor,
                                                              public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMSquareMagnitudeIntegralNormalizationPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual PostprocessorValue getValue() const override final;

private:
  /// Computed normalization constant
  mfem::real_t _norm{0.0};

  /// True when the named variable lives in the complex gridfunction container
  const bool _is_complex;

  /// Input variable, non-null when !_is_complex
  mfem::ParGridFunction * const _var;
  /// Input variable, non-null when _is_complex
  mfem::ParComplexGridFunction * const _cmplx_var;

  /// Auxiliary output variable, non-null when !_is_complex
  mfem::ParGridFunction * const _aux_var;
  /// Auxiliary output variable, non-null when _is_complex
  mfem::ParComplexGridFunction * const _cmplx_aux_var;

  /// Real variables
  std::unique_ptr<mfem::GridFunctionCoefficient> _u_coef;
  std::unique_ptr<mfem::ProductCoefficient> _u_sq_coef;

  /// Complex variables
  std::unique_ptr<mfem::GridFunctionCoefficient> _ur_coef;
  std::unique_ptr<mfem::GridFunctionCoefficient> _ui_coef;
  std::unique_ptr<mfem::ProductCoefficient> _ur_sq_coef;
  std::unique_ptr<mfem::ProductCoefficient> _ui_sq_coef;
  std::unique_ptr<mfem::SumCoefficient> _u_mag_sq_coef;

  std::unique_ptr<mfem::L2_FECollection> _l2_fec;
  std::unique_ptr<mfem::ParFiniteElementSpace> _test_fespace;
  std::unique_ptr<mfem::ParGridFunction> _test_fn;

  mfem::ConstantCoefficient _one;
  std::unique_ptr<mfem::ParLinearForm> _integrator;
};

#endif

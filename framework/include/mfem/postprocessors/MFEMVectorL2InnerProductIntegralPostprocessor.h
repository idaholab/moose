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
 * Compute the integral of the inner product between two MFEM vector FE variables, scaled by an
 * optional scalar coefficient.
 */
class MFEMVectorL2InnerProductIntegralPostprocessor : public MFEMPostprocessor,
                                                      public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMVectorL2InnerProductIntegralPostprocessor(const InputParameters & parameters);

  /**
   * Evaluate integral.
   */
  virtual void execute() override;

  /**
   * Return the last evaluated integral value.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  /// Value of the inner product integral
  mfem::real_t _integral;
  /// Reference to the first variable in the inner product
  mfem::ParGridFunction & _primal_var;
  /// Reference to the second variable in the inner product
  mfem::ParGridFunction & _dual_var;
  /// Scaling coefficient applied to the integrand
  mfem::Coefficient & _scalar_coef;
  /// Vector grid function coefficient representing the second variable
  mfem::VectorGridFunctionCoefficient _dual_var_coef;
  /// Scaled vector grid function coefficient representing the second variable
  mfem::ScalarVectorProductCoefficient _scaled_dual_var_coef;
  /// Handles the block restriction
  mfem::ParLinearForm _subdomain_integrator;
};

#endif

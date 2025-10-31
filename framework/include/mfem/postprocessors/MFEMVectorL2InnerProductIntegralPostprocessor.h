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
 * Compute the integral of the innter product between two MFEM vector FE variables, scaled by an
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
  mfem::real_t _integral;
  mfem::ParGridFunction & _primal_var;
  mfem::ParGridFunction & _dual_var;
  mfem::Coefficient & _scalar_coef;
  mfem::VectorGridFunctionCoefficient _dual_var_coef;
  mfem::ScalarVectorProductCoefficient _scaled_dual_var_coef;
  mfem::ParLinearForm _subdomain_integrator;
};

#endif

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
 * Compute the time average of the inner product between two complex MFEM vector FE variables,
 * scaled by an optional scalar coefficient.
 */
class MFEMComplexVectorTimeAveragedPostprocessor : public MFEMPostprocessor,
                                                   public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMComplexVectorTimeAveragedPostprocessor(const InputParameters & parameters);

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
  mfem::ParComplexGridFunction & _primal_var;
  mfem::ParComplexGridFunction & _dual_var;
  mfem::ParFiniteElementSpace _scalar_test_fespace;
  mfem::ParGridFunction _scalar_var;
  mfem::Coefficient & _scalar_coef;
  mfem::VectorGridFunctionCoefficient _primal_var_real_coef;
  mfem::VectorGridFunctionCoefficient _primal_var_imag_coef;
  mfem::VectorGridFunctionCoefficient _dual_var_real_coef;
  mfem::VectorGridFunctionCoefficient _dual_var_imag_coef;
  mfem::InnerProductCoefficient _real_inner_product_coef;
  mfem::InnerProductCoefficient _imag_inner_product_coef;
  mfem::SumCoefficient _sum_coef;
  mfem::ParLinearForm _subdomain_integrator;
};

#endif

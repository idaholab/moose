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
#include "MFEMBoundaryRestrictable.h"

/**
 * Compute the integral of the flux of an MFEM vector variable across a boundary,
 * scaled by an optional scalar coefficient.
 */
class MFEMVectorBoundaryFluxIntegralPostprocessor : public MFEMPostprocessor,
                                                    public MFEMBoundaryRestrictable
{
public:
  static InputParameters validParams();

  MFEMVectorBoundaryFluxIntegralPostprocessor(const InputParameters & parameters);

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
  mfem::ParGridFunction & _var;
  mfem::Coefficient & _scalar_coef;
  mfem::VectorGridFunctionCoefficient _var_coef;
  mfem::RT_FECollection _rt_fec;
  mfem::ParFiniteElementSpace _rt_vector_fespace;
  mfem::ParGridFunction _rt_var;
  mfem::ParLinearForm _boundary_integrator;
};

#endif

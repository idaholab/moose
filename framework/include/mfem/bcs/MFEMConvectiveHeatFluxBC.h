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
#include "MFEMIntegratedBC.h"

/**
 * \f[
 * (h (T-T_\infty), T')
 * \f]
 */
class MFEMConvectiveHeatFluxBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMConvectiveHeatFluxBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;

protected:
  mfem::Coefficient & _heat_transfer_coef;
  mfem::Coefficient & _T_inf_coef;
  mfem::ProductCoefficient & _external_heat_flux_coef;
};

#endif

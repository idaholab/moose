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
 * (h(T) (T-T_\infty), T')
 * \f]
 */
class MFEMNLConvectiveHeatFluxBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  /// Construct the nonlinear convective heat flux boundary condition.
  MFEMNLConvectiveHeatFluxBC(const InputParameters & parameters);

  /// Create MFEM non-linear integrator to apply to the LHS of the weak form. Ownership managed by the caller.
  virtual mfem::NonlinearFormIntegrator * createNLIntegrator() override;

protected:
  /// Heat transfer coefficient h(T).
  mfem::Coefficient & _heat_transfer_coef;
  /// Derivative dh/dT of the heat transfer coefficient.
  mfem::Coefficient & _d_heat_transfer_dT_coef;
  /// Far-field temperature T_inf(T).
  mfem::Coefficient & _T_inf_coef;
  /// Derivative dT_inf/dT of the far-field temperature coefficient.
  mfem::Coefficient & _d_T_inf_dT_coef;
  /// Trial variable temperature T.
  mfem::Coefficient & _T_coef;
};

#endif

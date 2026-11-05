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

class MFEMBoundaryFlowIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMBoundaryFlowIntegratedBC(const InputParameters & parameters);

  /// Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

  virtual bool isDGBC() const override { return true; }

protected:
  /// Boundary value f carried into the domain by the flow
  mfem::Coefficient & _coef;
  /// Velocity coefficient Q
  mfem::VectorCoefficient & _vec_coef;
  mfem::real_t _alpha;
  mfem::real_t _beta;
};

#endif

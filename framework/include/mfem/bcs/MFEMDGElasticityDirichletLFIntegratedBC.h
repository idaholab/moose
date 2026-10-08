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

class MFEMDGElasticityDirichletLFIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMDGElasticityDirichletLFIntegratedBC(const InputParameters & parameters);

  /// Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator() override;

  virtual bool isDGBC() const override { return true; }

protected:
  int _fe_order;
  /// Dirichlet data u_D
  mfem::VectorCoefficient & _vec_coef;
  mfem::Coefficient & _lambda;
  mfem::Coefficient & _mu;
  mfem::real_t _alpha;
  mfem::real_t _kappa;
};

#endif

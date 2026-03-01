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

class MFEMDGDiffusionBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMDGDiffusionBC(const InputParameters & parameters);

  /// Create MFEM integrator to apply to the LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createFaceBFIntegrator() override;

  mfem::LinearFormIntegrator * createLFIntegrator() override { return nullptr; };
  mfem::BilinearFormIntegrator * createBFIntegrator() override { return nullptr; };

protected:
  /// Name of (the test variable associated with) the weak form that the kernel is applied to.
  int _fe_order;
  mfem::ConstantCoefficient _one;
  mfem::ConstantCoefficient _zero;
  mfem::real_t _sigma;
  mfem::real_t _kappa;
};

#endif

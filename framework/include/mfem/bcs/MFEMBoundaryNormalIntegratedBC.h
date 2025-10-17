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

class MFEMBoundaryNormalIntegratedBC : public MFEMIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMBoundaryNormalIntegratedBC(const InputParameters & parameters);

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller.
  virtual std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *>
  createLFIntegrator() override;

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
  createBFIntegrator() override;

protected:
  mfem::VectorCoefficient & _vec_coef;

  const MFEMVectorCoefficientName & _vec_coef_imag_name;
  mfem::VectorCoefficient & _vec_coef_imag;
};

#endif

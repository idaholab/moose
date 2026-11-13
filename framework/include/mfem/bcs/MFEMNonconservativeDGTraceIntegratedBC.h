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

#include "MFEMDGTraceIntegratedBC.h"

class MFEMNonconservativeDGTraceIntegratedBC : public MFEMDGTraceIntegratedBC
{
public:
  static InputParameters validParams();

  MFEMNonconservativeDGTraceIntegratedBC(const InputParameters & parameters);

  /// Create MFEM integrator to apply to the LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBFIntegrator() override;
};

#endif

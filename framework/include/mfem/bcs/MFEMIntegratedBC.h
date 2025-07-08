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
#include "MFEMBoundaryCondition.h"

class MFEMIntegratedBC : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMIntegratedBC(const InputParameters & parameters);
  virtual ~MFEMIntegratedBC() = default;

  // Create a new MFEM integrator to apply to the RHS of the weak form. Ownership managed by the
  // caller. The first element of the pair corresponds to the real part of the integrator.
  // The second element corresponds to the imaginary part.
  virtual std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *>
  createLFIntegrator() = 0;

  // Create a new MFEM integrator to apply to LHS of the weak form. Ownership managed by the caller.
  virtual std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *>
  createBFIntegrator() = 0;

  /// Get name of the trial variable (gridfunction) the kernel acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }
};

#endif

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

class MFEMDGBoundaryCondition : public MFEMBoundaryCondition
{
public:
  static InputParameters validParams();

  MFEMDGBoundaryCondition(const InputParameters & parameters);
  virtual ~MFEMDGBoundaryCondition() = default;

  /// Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator() = 0;

  /// Create MFEM integrator to apply to the LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBFIntegrator() = 0;

  /// Get name of the trial variable (gridfunction) the kernel acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }
protected:
  /// Name of (the test variable associated with) the weak form that the kernel is applied to.
  int _fe_order;
  mfem::ConstantCoefficient _one;
  mfem::ConstantCoefficient _zero;
  mfem::real_t _sigma;
  mfem::real_t _kappa;
};

#endif
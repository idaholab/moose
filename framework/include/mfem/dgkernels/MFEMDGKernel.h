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

#include "MFEMBlockRestrictable.h"
#include "MFEMGeneralUserObject.h"

class MFEMDGKernel : public MFEMGeneralUserObject, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();
  MFEMDGKernel(const InputParameters & parameters);
  virtual ~MFEMDGKernel() = default;

  // TODO: rename these back to createLFIntegrator and createBFIntegrator
  virtual mfem::BilinearFormIntegrator * createBFIntegrator() { return nullptr; }
  virtual mfem::LinearFormIntegrator * createLFIntegrator() { return nullptr; }

  /// Get name of the test variable labelling the weak form this kernel is added to
  const VariableName & getTestVariableName() const { return _test_var_name; }

  /// Get name of the trial variable (gridfunction) the kernel acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const VariableName & getTrialVariableName() const { return _test_var_name; }

protected:
  /// Name of (the test variable associated with) the weak form that the kernel is applied to.
  const VariableName & _test_var_name;
  int _fe_order;
  mfem::ConstantCoefficient _one;
  mfem::ConstantCoefficient _zero;
  mfem::real_t _sigma;
  mfem::real_t _kappa;
};

#endif

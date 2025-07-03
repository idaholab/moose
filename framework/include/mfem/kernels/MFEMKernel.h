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

#include "MFEMGeneralUserObject.h"
#include "MFEMContainers.h"
#include "MFEMBlockRestrictable.h"

/**
 * Class to construct an MFEM integrator to apply to the equation system.
 */
class MFEMKernel : public MFEMGeneralUserObject, public MFEMBlockRestrictable
{
public:
  static InputParameters validParams();

  MFEMKernel(const InputParameters & parameters);

  virtual ~MFEMKernel() = default;

  // Create a new MFEM integrator to apply to the weak form. Ownership managed by the caller.
  // The first element of the pair corresponds to the real part of the integrator. The second element corresponds to the imaginary part.
  virtual std::pair<mfem::LinearFormIntegrator *, mfem::LinearFormIntegrator *> createLFIntegrator() { return std::make_pair(nullptr, nullptr); }
  virtual std::pair<mfem::BilinearFormIntegrator *, mfem::BilinearFormIntegrator *> createBFIntegrator() { return std::make_pair(nullptr, nullptr); }

  /// Get name of the test variable labelling the weak form this kernel is added to
  const VariableName & getTestVariableName() const { return _test_var_name; }

  /// Get name of the trial variable (gridfunction) the kernel acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const VariableName & getTrialVariableName() const { return _test_var_name; }

protected:
  /// Name of (the test variable associated with) the weak form that the kernel is applied to.
  const VariableName & _test_var_name;
};

#endif

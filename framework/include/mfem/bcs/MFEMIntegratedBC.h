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

namespace Moose::MFEM
{
class IntegratedBC : public BoundaryCondition
{
public:
  static InputParameters validParams();

  IntegratedBC(const InputParameters & parameters);
  virtual ~IntegratedBC() = default;

  /// Create MFEM integrator to apply to the RHS of the weak form. Ownership managed by the caller.
  virtual mfem::LinearFormIntegrator * createLFIntegrator() { return nullptr; };

  /// Create MFEM non-linear integrator to apply to the LHS of the weak form. Ownership managed by the caller.
  virtual mfem::NonlinearFormIntegrator * createNLIntegrator() { return nullptr; };

  /// Create MFEM integrator to apply to the LHS of the weak form. Ownership managed by the caller.
  virtual mfem::BilinearFormIntegrator * createBFIntegrator() { return nullptr; };

  /// Get name of the trial variable (gridfunction) the bc acts on.
  /// Defaults to the name of the test variable labelling the weak form.
  virtual const std::string & getTrialVariableName() const { return _test_var_name; }
};

} // namespace Moose::MFEM
#endif

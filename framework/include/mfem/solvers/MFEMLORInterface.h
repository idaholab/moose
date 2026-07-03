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

#include "MFEMSolverBase.h"
#include "EquationSystem.h"

class MFEMProblemSolve;

namespace Moose::MFEM
{
/**
 * Base class for linear MFEM solvers and preconditioners.
 */
class LORInterface
{
public:
  static InputParameters validParams();

  LORInterface(LinearSolverBase & solver_base);

  /// Rebuild any Low-Order-Refined components from the unreduced bilinear form. Called only when
  /// IsLOR() is true, before the assembled linear operator has been set via SetOperator. Default
  /// no-op; override in solvers or preconditioners that construct LOR-related data from the
  /// bilinear form.
  virtual void SetupLOR() {}

protected:
  /// Checks for the correct configuration of quadrature bases for LOR spectral equivalence
  virtual void CheckSpectralEquivalence(mfem::ParBilinearForm & blf) const;

  /// Reference to associated base linear solver object
  LinearSolverBase & _solver_base;
    
  /// Variable defining whether to use LOR solver
  bool _lor;
};
} // namespace Moose::MFEM

#endif

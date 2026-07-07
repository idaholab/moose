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

#include "MFEMLinearSolverBase.h"

namespace Moose::MFEM
{
/**
 * Wrapper for mfem::MatrixFreeAMS solver that creates an mfem::MatrixFreeAMS solver from the
 * operator when set.
 */
class MatrixFreeAMS : public mfem::Solver
{
public:
  MatrixFreeAMS(mfem::Coefficient & alpha_coef,
                mfem::Coefficient & beta_coef,
                int inner_pi_its = 0,
                int inner_g_its = 1);

  /// Set the bilinear form corresponding to the curl-curl problem being preconditioned
  void SetBilinearForm(mfem::ParBilinearForm & a) { _aform = &a; }
  /// Set the marker array labelling essential boundaries
  void SetBoundaryMarkers(mfem::Array<int> & ess_bdr_markers)
  {
    _ess_bdr_markers = ess_bdr_markers;
  }
  void SetOperator(const mfem::Operator & op) override;
  void Mult(const mfem::Vector & x, mfem::Vector & y) const override
  {
    _matrix_free_ams->Mult(x, y);
  }

private:
  std::unique_ptr<mfem::MatrixFreeAMS> _matrix_free_ams{nullptr};
  mfem::Coefficient & _alpha_coef;
  mfem::Coefficient & _beta_coef;
  const int _inner_pi_its;
  const int _inner_g_its;
  mfem::ParBilinearForm * _aform;
  mfem::Array<int> _ess_bdr_markers;
};
} // namespace Moose::MFEM

namespace Moose::MFEM
{
/**
 * Base class for LOR compatible linear MFEM solvers and preconditioners.
 */
template <class MFEMSolverType>
class LORLinearSolverBase : public LinearSolverBase
{
public:
  static InputParameters validParams();

  LORLinearSolverBase(const InputParameters & parameters);

  /// Update the wrapped MFEM solver parameters
  virtual void SetSolverParameters(MFEMSolverType & solver) = 0;

  virtual void Update() override;

  /// Returns whether or not this solver (or its preconditioner) uses LOR
  bool IsLOR(LinearSolverBase & solver) const;

  /// Returns a pointer to the provided LOR interface of the solver's preconditioner if present.
  LORLinearSolverBase * GetPreconditionerLORInterface(LinearSolverBase & solver) const;

  /// Rebuild any Low-Order-Refined components from the unreduced bilinear form. Called only when
  /// IsLOR() is true, before the assembled linear operator has been set via SetOperator. Default
  /// no-op; override in solvers or preconditioners that construct LOR-related data from the
  /// bilinear form.
  virtual void SetupLOR(Moose::MFEM::EquationSystem & equation_system);

  void SetupLOR(LinearSolverBase & solver, Moose::MFEM::EquationSystem & equation_system);

  /// Update LOR solver following changes to the EquationSystem
  // void LORInterfaceUpdate(LinearSolverBase & solver, Moose::MFEM::EquationSystem & equation_system);

protected:
  /// Checks for the correct configuration of quadrature bases for LOR spectral equivalence
  virtual void CheckSpectralEquivalence(mfem::ParBilinearForm & blf) const;

  /// Variable defining whether to use LOR solver
  bool _lor;
  mfem::ParBilinearForm * _a;
  mfem::Array<int> _ess_bdr_markers;
  mfem::Array<int> _ess_tdofs;
};

// Template specializations required for LOR wrappers for Hypre iterative solvers that lack default
// constructors
template <>
void LORLinearSolverBase<mfem::HypreGMRES>::SetupLOR(LinearSolverBase & solver_base,
                                              Moose::MFEM::EquationSystem & equation_system);

template <>
void LORLinearSolverBase<mfem::HypreFGMRES>::SetupLOR(LinearSolverBase & solver_base,
                                               Moose::MFEM::EquationSystem & equation_system);

template <>
void LORLinearSolverBase<mfem::HyprePCG>::SetupLOR(LinearSolverBase & solver_base,
                                            Moose::MFEM::EquationSystem & equation_system);

template <>
void LORLinearSolverBase<MatrixFreeAMS>::SetupLOR(LinearSolverBase & solver_base,
                                                               Moose::MFEM::EquationSystem & equation_system);
} // namespace Moose::MFEM

#endif

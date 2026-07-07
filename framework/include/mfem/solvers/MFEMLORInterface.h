// //* This file is part of the MOOSE framework
// //* https://mooseframework.inl.gov
// //*
// //* All rights reserved, see COPYRIGHT for full restrictions
// //* https://github.com/idaholab/moose/blob/master/COPYRIGHT
// //*
// //* Licensed under LGPL 2.1, please see LICENSE for details
// //* https://www.gnu.org/licenses/lgpl-2.1.html

// #ifdef MOOSE_MFEM_ENABLED

// #pragma once

// #include "MFEMLinearSolverBase.h"
// #include "EquationSystem.h"

// class MFEMProblemSolve;

// namespace Moose::MFEM
// {
// /**
//  * Interface for LOR compatible linear MFEM solvers and preconditioners.
//  */
// class LORInterface
// {
// public:
//   static InputParameters validParams();

//   LORInterface(const InputParameters & parameters);

//   /// Returns whether or not this solver (or its preconditioner) uses LOR
//   bool IsLOR(LinearSolverBase & solver) const;

//   /// Returns a pointer to the provided LOR interface of the solver's preconditioner if present.
//   LORInterface * GetPreconditionerLORInterface(LinearSolverBase & solver) const;

//   /// Rebuild any Low-Order-Refined components from the unreduced bilinear form. Called only when
//   /// IsLOR() is true, before the assembled linear operator has been set via SetOperator. Default
//   /// no-op; override in solvers or preconditioners that construct LOR-related data from the
//   /// bilinear form.
//   virtual void SetupLOR(Moose::MFEM::EquationSystem & equation_system);

//   template <typename MFEMSolverType>
//   void SetupLOR(LinearSolverBase & solver, Moose::MFEM::EquationSystem & equation_system);

//   /// Update LOR solver following changes to the EquationSystem
//   template <typename MFEMSolverType>
//   void Update(LinearSolverBase & solver, Moose::MFEM::EquationSystem & equation_system);

// protected:
//   /// Checks for the correct configuration of quadrature bases for LOR spectral equivalence
//   virtual void CheckSpectralEquivalence(mfem::ParBilinearForm & blf) const;

//   /// Variable defining whether to use LOR solver
//   bool _lor;
//   mfem::ParBilinearForm * _a;
//   mfem::Array<int> _ess_bdr_markers;
//   mfem::Array<int> _ess_tdofs;
// };

// // Template specializations required for LOR wrappers for Hypre iterative solvers that lack default
// // constructors
// template <>
// void LORInterface::SetupLOR<mfem::HypreGMRES>(LinearSolverBase & solver_base,
//                                               Moose::MFEM::EquationSystem & equation_system);

// template <>
// void LORInterface::SetupLOR<mfem::HypreFGMRES>(LinearSolverBase & solver_base,
//                                                Moose::MFEM::EquationSystem & equation_system);

// template <>
// void LORInterface::SetupLOR<mfem::HyprePCG>(LinearSolverBase & solver_base,
//                                             Moose::MFEM::EquationSystem & equation_system);

// } // namespace Moose::MFEM

// #endif

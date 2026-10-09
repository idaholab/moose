//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMOperatorJacobiSmoother.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMOperatorJacobiSmoother);

InputParameters
MFEMOperatorJacobiSmoother::validParams()
{
  InputParameters params =
      Moose::MFEM::LORLinearSolverBase<mfem::OperatorJacobiSmoother>::validParams();
  params.addClassDescription("MFEM solver for performing Jacobi smoothing of the equation system.");
  params.addParam<mfem::real_t>(
      "damping",
      1.0,
      "Damping factor omega for the scaled-Jacobi iteration y = omega * D^{-1} * x. "
      "When used as a multigrid smoother, omega must satisfy omega < 2/lambda_max(D^{-1}A).");
  params.addParam<VariableName>(
      "schur_complement_variable",
      "If set, precondition the diagonal block of this variable with the diagonal of the "
      "approximate Schur complement B diag(M)^{-1} B^T instead of the block itself. For "
      "saddle-point systems whose diagonal block for this variable is zero. Requires "
      "'schur_complement_coupled_variable', partial assembly, and use as a sub-preconditioner "
      "of MFEMBlockDiagonalPreconditioner.");
  params.addParam<VariableName>(
      "schur_complement_coupled_variable",
      "Variable whose diagonal block is M in the Schur complement approximation. B is the "
      "block coupling it into the equation for 'schur_complement_variable'.");
  return params;
}

MFEMOperatorJacobiSmoother::MFEMOperatorJacobiSmoother(const InputParameters & parameters)
  : Moose::MFEM::LORLinearSolverBase<mfem::OperatorJacobiSmoother>(parameters),
    _use_schur_complement(isParamValid("schur_complement_variable"))
{
  if (_use_schur_complement != isParamValid("schur_complement_coupled_variable"))
    paramError("schur_complement_variable",
               "'schur_complement_variable' and 'schur_complement_coupled_variable' must be set "
               "together.");

  // Low-order refinement replaces the smoother built from the Schur complement diagonal
  if (_use_schur_complement && _lor)
    paramError("schur_complement_variable",
               "The Schur complement approximation is not supported with low-order refinement.");

  ConstructSolver();
}

void
MFEMOperatorJacobiSmoother::ConstructSolver()
{
  auto solver = std::make_unique<mfem::OperatorJacobiSmoother>(getParam<double>("damping"));
  SetSolverParameters(*solver);
  _solver = std::move(solver);
}

void
MFEMOperatorJacobiSmoother::SetSolverParameters(mfem::OperatorJacobiSmoother & solver)
{
  solver.iterative_mode = getParam<bool>("use_initial_guess");
}

void
MFEMOperatorJacobiSmoother::SetOperatorImpl(mfem::Operator & op)
{
  // non-schur mode - fall back to normal route
  if (!_use_schur_complement)
    return GetSolver().SetOperator(op);

  // schur mode
  if (_equation_system->GetAssemblyLevel() != mfem::AssemblyLevel::PARTIAL)
    mooseError("Tried to use Schur complement on Jacobi smoother, but Equation System is not set "
               "to partial assembly.");

  const auto & var_name = getParam<VariableName>("schur_complement_variable");
  const auto & coupled_var_name = getParam<VariableName>("schur_complement_coupled_variable");

  auto & m_form = _equation_system->GetBilinearForm(coupled_var_name);
  mfem::Vector diag_m(m_form.ParFESpace()->GetTrueVSize());
  m_form.AssembleDiagonal(diag_m);
  diag_m.Reciprocal();

  // Constrained B has the columns of the coupled variable's essential DoFs removed, so they
  // contribute nothing to the Schur complement
  mfem::Array<int> coupled_ess_tdofs;
  m_form.ParFESpace()->GetEssentialTrueDofs(
      _equation_system->GetEssentialBoundaryMarkers(coupled_var_name), coupled_ess_tdofs);
  diag_m.SetSubVector(coupled_ess_tdofs, 0.0);

  // Diagonal of B diag(M)^{-1} B^T on the true DoFs of var_name
  mfem::Vector diag_s(op.Height());
  _equation_system->GetMixedBilinearForm(var_name, coupled_var_name)
      .AssembleDiagonal_ADAt(diag_m, diag_s);

  _equation_system->GetGridFunction(var_name).ParFESpace()->GetEssentialTrueDofs(
      _equation_system->GetEssentialBoundaryMarkers(var_name), _ess_tdofs);

  // Constructed from a diagonal, mfem::OperatorJacobiSmoother never calls AssembleDiagonal on op,
  // whose diagonal is zero for a saddle-point system
  auto solver = std::make_unique<mfem::OperatorJacobiSmoother>(
      diag_s, _ess_tdofs, getParam<double>("damping"));
  SetSolverParameters(*solver);
  _solver = std::move(solver);
}

#endif

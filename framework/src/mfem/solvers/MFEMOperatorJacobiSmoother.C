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
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("MFEM solver for performing Jacobi smoothing of the equation system.");
  params.addParam<double>(
      "damping",
      1.0,
      "Damping factor omega for the scaled-Jacobi iteration y = omega * D^{-1} * x. "
      "When used as a multigrid smoother, omega must satisfy omega < 2/lambda_max(D^{-1}A).");
  return params;
}

MFEMOperatorJacobiSmoother::MFEMOperatorJacobiSmoother(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters)
{
  ConstructSolver();
}

void
MFEMOperatorJacobiSmoother::ConstructSolver()
{
  _solver = std::make_unique<mfem::OperatorJacobiSmoother>(getParam<double>("damping"));
  _solver->iterative_mode = getParam<bool>("use_initial_guess");
}

void
MFEMOperatorJacobiSmoother::SetupLOR(mfem::ParBilinearForm & a, mfem::Array<int> & ess_bdr_markers)
{
  if (_lor)
  {
    CheckSpectralEquivalence(a);
    mfem::Array<int> ess_tdofs;
    a.ParFESpace()->GetEssentialTrueDofs(ess_bdr_markers, ess_tdofs);
    _solver.reset(new mfem::LORSolver<mfem::OperatorJacobiSmoother>(a, ess_tdofs));
  }
}

#endif

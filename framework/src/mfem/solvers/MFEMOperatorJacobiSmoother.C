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

registerMooseMFEMObject("MooseApp", OperatorJacobiSmoother);

namespace Moose::MFEM
{
InputParameters
OperatorJacobiSmoother::validParams()
{
  InputParameters params = LinearSolverBase::validParams();
  params.addClassDescription("MFEM solver for performing Jacobi smoothing of the equation system.");
  params.addParam<double>(
      "damping",
      1.0,
      "Damping factor omega for the scaled-Jacobi iteration y = omega * D^{-1} * x. "
      "When used as a multigrid smoother, omega must satisfy omega < 2/lambda_max(D^{-1}A).");
  return params;
}

OperatorJacobiSmoother::OperatorJacobiSmoother(const InputParameters & parameters)
  : LinearSolverBase(parameters)
{
  constructSolver();
}

void
OperatorJacobiSmoother::constructSolver()
{
  _solver = std::make_unique<mfem::OperatorJacobiSmoother>(getParam<double>("damping"));
}

void
OperatorJacobiSmoother::setupLOR(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (_lor)
  {
    checkSpectralEquivalence(a);
    _solver.reset(new mfem::LORSolver<mfem::OperatorJacobiSmoother>(a, tdofs));
  }
}

} // namespace Moose::MFEM
#endif

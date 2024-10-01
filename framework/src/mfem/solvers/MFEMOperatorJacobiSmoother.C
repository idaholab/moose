#pragma once
#include "MFEMOperatorJacobiSmoother.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMOperatorJacobiSmoother);

InputParameters
MFEMOperatorJacobiSmoother::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();

  return params;
}

MFEMOperatorJacobiSmoother::MFEMOperatorJacobiSmoother(const InputParameters & parameters)
  : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMOperatorJacobiSmoother::constructSolver(const InputParameters & parameters)
{
  _preconditioner = std::make_shared<mfem::OperatorJacobiSmoother>(
      getMFEMProblem().mesh().getMFEMParMesh().GetComm());
}
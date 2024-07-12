#pragma once
#include "MFEMHyprePCGSolver.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMHyprePCGSolver);

InputParameters
MFEMHyprePCGSolver::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();

  params.addParam<double>("tolerance", 1e-16, "Set the tolerance.");
  params.addParam<double>("abs_tolerance", 1e-16, "Set the absolute tolerance.");
  params.addParam<int>("max_iteration", 1000, "Set the maximum iteration.");

  // NB: - will require option to add a preconditioner somewhere.

  return params;
}

MFEMHyprePCGSolver::MFEMHyprePCGSolver(const InputParameters & parameters)
  : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMHyprePCGSolver::constructSolver(const InputParameters & parameters)
{
  _solver = std::make_shared<mfem::HyprePCG>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());

  // Set solver options.
  _solver->SetTol(parameters.get<double>("tolerance"));
  _solver->SetAbsTol(parameters.get<double>("abs_tolerance"));
  _solver->SetMaxIter(parameters.get<int>("max_iteration"));

  // NB: - set the preconditioner here.
}
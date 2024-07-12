#pragma once
#include "MFEMHypreFGMRESSolver.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMHypreFGMRESSolver);

InputParameters
MFEMHypreFGMRESSolver::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();

  params.addParam<double>("tolerance", 1e-16, "Set the tolerance.");
  params.addParam<int>("max_iteration", 1000, "Set the maximum iteration.");
  params.addParam<int>("kdim", 10, "Set the k-dimension.");

  // NB: - will require option to add a preconditioner somewhere.

  return params;
}

MFEMHypreFGMRESSolver::MFEMHypreFGMRESSolver(const InputParameters & parameters)
  : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMHypreFGMRESSolver::constructSolver(const InputParameters & parameters)
{
  _solver = std::make_shared<mfem::HypreFGMRES>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());

  // Set solver options.
  _solver->SetTol(parameters.get<double>("tolerance"));
  _solver->SetMaxIter(parameters.get<int>("max_iteration"));
  _solver->SetKDim(parameters.get<int>("kdim"));

  // NB: - set the preconditioner here.
}
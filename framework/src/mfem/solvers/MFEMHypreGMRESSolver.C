#pragma once
#include "MFEMHypreGMRESSolver.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMHypreGMRESSolver);

InputParameters
MFEMHypreGMRESSolver::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();

  params.addParam<double>("tolerance", 1e-16, "Set the tolerance.");
  params.addParam<double>("abs_tolerance", 1e-16, "Set the absolute tolerance.");
  params.addParam<int>("max_iteration", 1000, "Set the maximum iteration.");
  params.addParam<int>("kdim", 10, "Set the k-dimension.");

  // NB: - will require option to add a preconditioner somewhere.

  return params;
}

MFEMHypreGMRESSolver::MFEMHypreGMRESSolver(const InputParameters & parameters)
  : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMHypreGMRESSolver::constructSolver(const InputParameters & parameters)
{
  _solver = std::make_shared<mfem::HypreGMRES>(getMFEMProblem().getProblemData()._comm);

  // Set solver options.
  _solver->SetTol(parameters.get<double>("tolerance"));
  _solver->SetAbsTol(parameters.get<double>("abs_tolerance"));
  _solver->SetMaxIter(parameters.get<int>("max_iteration"));
  _solver->SetKDim(parameters.get<int>("kdim"));

  // NB: - set the preconditioner here.
}
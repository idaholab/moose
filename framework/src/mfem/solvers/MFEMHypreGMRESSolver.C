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
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");
  // NB: - will require option to add a preconditioner somewhere.

  return params;
}

MFEMHypreGMRESSolver::MFEMHypreGMRESSolver(const InputParameters & parameters)
  : MFEMSolverBase(parameters),
    _preconditioner(getUserObject<MFEMPreconditionerBase>("preconditioner"))
{
  constructSolver(parameters);
}

void
MFEMHypreGMRESSolver::constructSolver(const InputParameters & parameters)
{
  auto hypre_preconditioner =
      std::dynamic_pointer_cast<mfem::HypreSolver>(_preconditioner.getPreconditioner());
  _solver = std::make_shared<mfem::HypreGMRES>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());
  EquationSystems & es = getMFEMProblem().es();

  // Set solver options.
  _solver->SetTol(float(es.parameters.get<Real>("linear solver tolerance")));
  _solver->SetAbsTol(float(es.parameters.get<Real>("linear solver absolute tolerance")));
  _solver->SetMaxIter(es.parameters.get<unsigned int>("linear solver maximum iterations"));
  _solver->SetKDim(parameters.get<int>("kdim"));

  if (hypre_preconditioner)
    _solver->SetPreconditioner(*hypre_preconditioner);

  // // Set solver options.
  // _solver->SetTol(parameters.get<double>("tolerance"));
  // _solver->SetAbsTol(parameters.get<double>("abs_tolerance"));
  // _solver->SetMaxIter(parameters.get<int>("max_iteration"));
  // _solver->SetKDim(parameters.get<int>("kdim"));
}
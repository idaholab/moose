#pragma once
#include "MFEMHypreGMRESSolver.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMHypreGMRESSolver);

InputParameters
MFEMHypreGMRESSolver::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();

  params.addParam<double>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<double>("l_abs_tol", 1e-50, "Set the absolute tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("kdim", 10, "Set the k-dimension.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");

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
  _solver->SetTol(getParam<double>("l_tol"));
  _solver->SetAbsTol(getParam<double>("l_abs_tol"));
  _solver->SetMaxIter(getParam<int>("l_max_its"));
  _solver->SetKDim(getParam<int>("kdim"));
  _solver->SetPrintLevel(getParam<int>("print_level"));

  if (hypre_preconditioner)
    _solver->SetPreconditioner(*hypre_preconditioner);
}
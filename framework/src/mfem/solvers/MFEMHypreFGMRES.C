#pragma once
#include "MFEMHypreFGMRES.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMHypreFGMRES);

InputParameters
MFEMHypreFGMRES::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("Hypre solver for the iterative solution of MFEM equation systems "
                             "using the flexible generalized minimal residual method.");
  params.addParam<double>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("kdim", 10, "Set the k-dimension.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMHypreFGMRES::MFEMHypreFGMRES(const InputParameters & parameters)
  : MFEMSolverBase(parameters),
    _preconditioner(isParamSetByUser("preconditioner")
                        ? getUserObject<MFEMSolverBase>("preconditioner").getSolver()
                        : nullptr)
{
  constructSolver(parameters);
}

void
MFEMHypreFGMRES::constructSolver(const InputParameters &)
{
  auto hypre_preconditioner = std::dynamic_pointer_cast<mfem::HypreSolver>(_preconditioner);

  _solver = std::make_shared<mfem::HypreFGMRES>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());
  _solver->SetTol(getParam<double>("l_tol"));
  _solver->SetMaxIter(getParam<int>("l_max_its"));
  _solver->SetKDim(getParam<int>("kdim"));
  _solver->SetPrintLevel(getParam<int>("print_level"));

  if (hypre_preconditioner)
    _solver->SetPreconditioner(*hypre_preconditioner);
}

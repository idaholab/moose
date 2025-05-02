#ifdef MFEM_ENABLED

#include "MFEMHypreFGMRES.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreFGMRES);

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
  _preconditioner(
    isParamSetByUser("preconditioner")
        ? getMFEMProblem().getProblemData().mfem_preconditioner
        : nullptr)
{
  constructSolver(parameters);
}

void
MFEMHypreFGMRES::constructSolver(const InputParameters &)
{

  _jacobian_solver = std::make_shared<mfem::HypreFGMRES>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());
  _jacobian_solver->SetTol(getParam<double>("l_tol"));
  _jacobian_solver->SetMaxIter(getParam<int>("l_max_its"));
  _jacobian_solver->SetKDim(getParam<int>("kdim"));
  _jacobian_solver->SetPrintLevel(getParam<int>("print_level"));

  if (_preconditioner)
  {
    auto hypre_preconditioner = std::dynamic_pointer_cast<mfem::HypreSolver>(_preconditioner->getSolver());
    _jacobian_solver->SetPreconditioner(*hypre_preconditioner);
  }
  
  _solver = std::dynamic_pointer_cast<mfem::Solver>(_jacobian_solver);
}

void
MFEMHypreFGMRES::updateSolver(mfem::ParBilinearForm &a, mfem::Array<int> &tdofs)
{
  bool lor = getParam<bool>("low_order_refined");
  
  mooseAssert(!(lor && _preconditioner), "LOR solver cannot take a preconditioner");

  if (_preconditioner)
  {
    _preconditioner->updateSolver(a,tdofs);
    auto hypre_preconditioner = std::dynamic_pointer_cast<mfem::HypreSolver>(_preconditioner->getSolver());
    _jacobian_solver->SetPreconditioner(*hypre_preconditioner);
    _solver = std::dynamic_pointer_cast<mfem::Solver>(_jacobian_solver);
  }
  else if (lor)
  {
    mooseError("HypreFGMRES solver does not support LOR solve");
  }
    
}

#endif

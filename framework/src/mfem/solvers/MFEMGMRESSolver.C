#ifdef MFEM_ENABLED

#include "MFEMGMRESSolver.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMGMRESSolver);

InputParameters
MFEMGMRESSolver::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("MFEM native solver for the iterative solution of MFEM equation "
                             "systems using the generalized minimal residual method.");

  params.addParam<double>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<double>("l_abs_tol", 1e-50, "Set the absolute tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMGMRESSolver::MFEMGMRESSolver(const InputParameters & parameters)
  : MFEMSolverBase(parameters),
    _preconditioner(isParamSetByUser("preconditioner")
                        ? getMFEMProblem().getProblemData().jacobian_preconditioner
                        : nullptr)
{
  constructSolver(parameters);
}

void
MFEMGMRESSolver::constructSolver(const InputParameters &)
{
  _jacobian_solver =
      std::make_shared<mfem::GMRESSolver>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());
  _jacobian_solver->SetRelTol(getParam<double>("l_tol"));
  _jacobian_solver->SetAbsTol(getParam<double>("l_abs_tol"));
  _jacobian_solver->SetMaxIter(getParam<int>("l_max_its"));
  _jacobian_solver->SetPrintLevel(getParam<int>("print_level"));

  if (_preconditioner)
    _jacobian_solver->SetPreconditioner(*_preconditioner->getSolver());

  _solver = std::dynamic_pointer_cast<mfem::Solver>(_jacobian_solver);
}

void
MFEMGMRESSolver::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  bool lor = getParam<bool>("low_order_refined");

  if (lor && _preconditioner)
    mooseError("LOR solver cannot take a preconditioner");

  if (_preconditioner)
  {
    _preconditioner->updateSolver(a, tdofs);
    _jacobian_solver->SetPreconditioner(*_preconditioner->getSolver());
    _solver = std::dynamic_pointer_cast<mfem::Solver>(_jacobian_solver);
  }
  else if (lor)
  {
    auto lor_solver = new mfem::LORSolver<mfem::GMRESSolver>(a, tdofs);
    lor_solver->GetSolver().SetRelTol(getParam<double>("l_tol"));
    lor_solver->GetSolver().SetAbsTol(getParam<double>("l_abs_tol"));
    lor_solver->GetSolver().SetMaxIter(getParam<int>("l_max_its"));
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));

    _solver.reset(lor_solver);
  }
}

#endif

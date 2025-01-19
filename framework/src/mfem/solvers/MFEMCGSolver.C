#ifdef MFEM_ENABLED

#include "MFEMCGSolver.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMCGSolver);

InputParameters
MFEMCGSolver::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("MFEM native solver for the iterative solution of MFEM equation "
                             "systems using the conjugate gradient method.");

  params.addParam<double>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<double>("l_abs_tol", 1e-50, "Set the absolute tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMCGSolver::MFEMCGSolver(const InputParameters & parameters)
  : MFEMSolverBase(parameters),
    _preconditioner(isParamSetByUser("preconditioner")
                        ? getUserObject<MFEMSolverBase>("preconditioner").getSolver()
                        : nullptr)
{
  constructSolver(parameters);
}

void
MFEMCGSolver::constructSolver(const InputParameters &)
{
  auto preconditioner = std::dynamic_pointer_cast<mfem::Solver>(_preconditioner);

  _solver = std::make_shared<mfem::CGSolver>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());
  _solver->SetRelTol(getParam<double>("l_tol"));
  _solver->SetAbsTol(getParam<double>("l_abs_tol"));
  _solver->SetMaxIter(getParam<int>("l_max_its"));
  _solver->SetPrintLevel(getParam<int>("print_level"));

  if (preconditioner)
    _solver->SetPreconditioner(*preconditioner);
}

#endif

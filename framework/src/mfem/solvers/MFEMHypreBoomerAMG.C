#ifdef MFEM_ENABLED

#include "MFEMHypreBoomerAMG.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreBoomerAMG);

InputParameters
MFEMHypreBoomerAMG::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("Hypre BoomerAMG solver and preconditioner for the iterative solution "
                             "of MFEM equation systems.");
  params.addParam<double>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<UserObjectName>(
      "fespace", "H1 FESpace to use in HypreBoomerAMG setup for elasticity problems.");
  params.addParam<mfem::real_t>(
      "strength_threshold", 0.25, "HypreBoomerAMG strong threshold. Defaults to 0.25.");
  return params;
}

MFEMHypreBoomerAMG::MFEMHypreBoomerAMG(const InputParameters & parameters)
  : MFEMSolverBase(parameters),
    _mfem_fespace(isParamSetByUser("fespace") ? getUserObject<MFEMFESpace>("fespace").getFESpace()
                                              : nullptr),
    _strength_threshold(getParam<mfem::real_t>("strength_threshold"))
{
  constructSolver(parameters);
}

void
MFEMHypreBoomerAMG::constructSolver(const InputParameters &)
{
  _jacobian_solver = std::make_shared<mfem::HypreBoomerAMG>();

  _jacobian_solver->SetTol(getParam<double>("l_tol"));
  _jacobian_solver->SetMaxIter(getParam<int>("l_max_its"));
  _jacobian_solver->SetPrintLevel(getParam<int>("print_level"));
  _jacobian_solver->SetStrengthThresh(_strength_threshold);

  if (_mfem_fespace && !mfem::HypreUsingGPU())
    _jacobian_solver->SetElasticityOptions(_mfem_fespace.get());

  _solver = _jacobian_solver;
}

void
MFEMHypreBoomerAMG::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{

  if (_lor)
  {
    auto lor_solver = new mfem::LORSolver<mfem::HypreBoomerAMG>(a, tdofs);
    lor_solver->GetSolver().SetTol(getParam<double>("l_tol"));
    lor_solver->GetSolver().SetMaxIter(getParam<int>("l_max_its"));
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));
    lor_solver->GetSolver().SetStrengthThresh(_strength_threshold);

    if (_mfem_fespace && !mfem::HypreUsingGPU())
      lor_solver->GetSolver().SetElasticityOptions(_mfem_fespace.get());

    _solver.reset(lor_solver);
  }
}

#endif

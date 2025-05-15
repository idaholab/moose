#ifdef MFEM_ENABLED

#include "MFEMHypreADS.h"

registerMooseObject("MooseApp", MFEMHypreADS);

InputParameters
MFEMHypreADS::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("Hypre auxiliary-space divergence solver and preconditioner for the "
                             "iterative solution of MFEM equation systems.");
  params.addParam<UserObjectName>("fespace", "H(div) FESpace to use in HypreADS setup.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");

  return params;
}

MFEMHypreADS::MFEMHypreADS(const InputParameters & parameters)
  : MFEMSolverBase(parameters), _mfem_fespace(getUserObject<MFEMFESpace>("fespace"))
{
  constructSolver(parameters);
}

void
MFEMHypreADS::constructSolver(const InputParameters &)
{
  auto solver = std::make_shared<mfem::HypreADS>(_mfem_fespace.getFESpace().get());
  solver->SetPrintLevel(getParam<int>("print_level"));

  _solver = solver;
}

void
MFEMHypreADS::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (_lor)
  {
    auto lor_solver = new mfem::LORSolver<mfem::HypreADS>(a, tdofs);
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));
    _solver.reset(lor_solver);
  }
}

#endif

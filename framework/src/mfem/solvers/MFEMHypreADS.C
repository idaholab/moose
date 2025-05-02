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
  _jacobian_preconditioner = std::make_shared<mfem::HypreADS>(_mfem_fespace.getFESpace().get());
  _jacobian_preconditioner->SetPrintLevel(getParam<int>("print_level"));

  _preconditioner = std::dynamic_pointer_cast<mfem::Solver>(_jacobian_preconditioner);

}

void
MFEMHypreADS::updateSolver(mfem::ParBilinearForm &a, mfem::Array<int> &tdofs)
{

  if (getParam<bool>("low_order_refined"))
    _preconditioner.reset(new mfem::LORSolver<mfem::HypreADS>(a, tdofs));
  
}


#endif

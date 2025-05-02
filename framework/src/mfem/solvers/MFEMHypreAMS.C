#ifdef MFEM_ENABLED

#include "MFEMHypreAMS.h"

registerMooseObject("MooseApp", MFEMHypreAMS);

InputParameters
MFEMHypreAMS::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("Hypre auxiliary-space Maxwell solver and preconditioner for the "
                             "iterative solution of MFEM equation systems.");
  params.addParam<UserObjectName>("fespace", "H(curl) FESpace to use in HypreAMS setup.");
  params.addParam<bool>("singular",
                        false,
                        "Declare that the system is singular; use when solving curl-curl problem "
                        "if mass term is zero");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<bool>("low_order_refined", false, "Set usage of Low-Order Refined solver.");

  return params;
}

MFEMHypreAMS::MFEMHypreAMS(const InputParameters & parameters)
  : MFEMSolverBase(parameters), _mfem_fespace(getUserObject<MFEMFESpace>("fespace"))
{
  constructSolver(parameters);
}

void
MFEMHypreAMS::constructSolver(const InputParameters &)
{
  _jacobian_preconditioner = std::make_shared<mfem::HypreAMS>(_mfem_fespace.getFESpace().get());
  if (getParam<bool>("singular"))
    _jacobian_preconditioner->SetSingularProblem();

  _jacobian_preconditioner->SetPrintLevel(getParam<int>("print_level"));

  _preconditioner = std::dynamic_pointer_cast<mfem::Solver>(_jacobian_preconditioner);
}

void
MFEMHypreAMS::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{

  if (getParam<bool>("low_order_refined"))
    _preconditioner.reset(new mfem::LORSolver<mfem::HypreAMS>(a, tdofs));
}

#endif

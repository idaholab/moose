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
  auto solver = std::make_shared<mfem::HypreAMS>(_mfem_fespace.getFESpace().get());
  if (getParam<bool>("singular"))
    solver->SetSingularProblem();

  solver->SetPrintLevel(getParam<int>("print_level"));

  _solver = solver;
}

void
MFEMHypreAMS::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{

  if (_lor)
  {
    if (_mfem_fespace.getFESpace()->GetMesh()->GetElement(0)->GetGeometryType() != mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreAMS Solver only supports hex meshes.");

    auto lor_solver = new mfem::LORSolver<mfem::HypreAMS>(a, tdofs);
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));
    if (getParam<bool>("singular"))
      lor_solver->GetSolver().SetSingularProblem();

    _solver.reset(lor_solver);
  }
}

#endif

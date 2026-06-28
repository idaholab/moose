//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreAMS.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreAMS);

InputParameters
MFEMHypreAMS::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("Hypre auxiliary-space Maxwell solver and preconditioner for the "
                             "iterative solution of MFEM equation systems.");
  params.addParam<MFEMFESpaceName>("fespace", "H(curl) FESpace to use in HypreAMS setup.");
  params.addParam<bool>("singular",
                        false,
                        "Declare that the system is singular; use when solving curl-curl problem "
                        "if mass term is zero");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");

  return params;
}

MFEMHypreAMS::MFEMHypreAMS(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters),
    _mfem_fespace(getMFEMProblem().getMFEMObject<MFEMFESpace>("MFEMFESpace",
                                                              getParam<MFEMFESpaceName>("fespace")))
{
  ConstructSolver();
}

void
MFEMHypreAMS::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreAMS>(_mfem_fespace.getFESpace().get());
  if (getParam<bool>("singular"))
    solver->SetSingularProblem();

  solver->iterative_mode = getParam<bool>("use_initial_guess");
  solver->SetPrintLevel(getParam<int>("print_level"));

  _solver = std::move(solver);
}

void
MFEMHypreAMS::SetupLOR()
{
  if (_lor)
  {
    if (_equation_system->isComplex())
      mooseError("LOR solve is not supported for complex equation systems.");
    if (_equation_system->GetTestVarNames().size() > 1)
      mooseError("LOR solve is only supported for single-variable systems");

    const auto & test_var_name = _equation_system->GetTestVarNames().at(0);
    const auto & trial_var_name = _equation_system->GetTrialVarNames().at(0);
    mfem::ParBilinearForm & a = _equation_system->GetBilinearForm(test_var_name);
    mfem::ParGridFunction & trial_gf = *getMFEMProblem().getGridFunction(trial_var_name);

    mfem::Array<int> ess_bdr_markers(trial_gf.ParFESpace()->GetParMesh()->bdr_attributes.Max());
    ess_bdr_markers = 0;
    _equation_system->ApplyEssentialBC(trial_var_name, trial_gf, ess_bdr_markers);

    CheckSpectralEquivalence(a);
    if (_mfem_fespace.getFESpace()->GetMesh()->GetElement(0)->GetGeometryType() !=
        mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreAMS Solver only supports hex meshes.");

    mfem::Array<int> ess_tdofs;
    a.ParFESpace()->GetEssentialTrueDofs(ess_bdr_markers, ess_tdofs);
    auto lor_solver = new mfem::LORSolver<mfem::HypreAMS>(a, ess_tdofs);
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));
    if (getParam<bool>("singular"))
      lor_solver->GetSolver().SetSingularProblem();

    _solver.reset(lor_solver);
  }
}

#endif

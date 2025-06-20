//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  auto solver = std::make_unique<mfem::HypreADS>(_mfem_fespace.getFESpace().get());
  solver->SetPrintLevel(getParam<int>("print_level"));

  _solver = std::move(solver);
}

void
MFEMHypreADS::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (_lor)
  {
    if (!checkSpectralEquivalence(a))
      mooseError("Low-Order-Refined solver requires the FESpace closed_basis to be GaussLobatto "
                 "and the open-basis to be IntegratedGLL for ND and RT elements.");

    if (_mfem_fespace.getFESpace()->GetMesh()->GetElement(0)->GetGeometryType() !=
        mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreADS Solver only supports hex meshes.");

    auto lor_solver = new mfem::LORSolver<mfem::HypreADS>(a, tdofs);
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));
    _solver.reset(lor_solver);
  }
}

#endif

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreADS.h"
#include "MFEMProblem.h"

registerMooseMFEMObject("MooseApp", HypreADS);

namespace Moose::MFEM
{
InputParameters
HypreADS::validParams()
{
  InputParameters params = LinearSolverBase::validParams();
  params.addClassDescription("Hypre auxiliary-space divergence solver and preconditioner for the "
                             "iterative solution of MFEM equation systems.");
  params.addParam<Moose::MFEM::FESpaceName>("fespace", "H(div) FESpace to use in HypreADS setup.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");

  return params;
}

HypreADS::HypreADS(const InputParameters & parameters)
  : LinearSolverBase(parameters),
    _mfem_fespace(getMFEMProblem().getMFEMObject<FESpace>(
        "Moose::MFEM::FESpace", getParam<Moose::MFEM::FESpaceName>("fespace")))
{
  constructSolver();
}

void
HypreADS::constructSolver()
{
  auto solver = std::make_unique<mfem::HypreADS>(_mfem_fespace.getFESpace().get());
  solver->SetPrintLevel(getParam<int>("print_level"));

  _solver = std::move(solver);
}

void
HypreADS::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (_lor)
  {
    checkSpectralEquivalence(a);
    if (_mfem_fespace.getFESpace()->GetMesh()->GetElement(0)->GetGeometryType() !=
        mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreADS Solver only supports hex meshes.");

    auto lor_solver = new mfem::LORSolver<mfem::HypreADS>(a, tdofs);
    lor_solver->GetSolver().SetPrintLevel(getParam<int>("print_level"));
    _solver.reset(lor_solver);
  }
}

} // namespace Moose::MFEM
#endif

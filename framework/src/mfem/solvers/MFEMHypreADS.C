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

registerMooseObject("MooseApp", MFEMHypreADS);

InputParameters
MFEMHypreADS::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("Hypre auxiliary-space divergence solver and preconditioner for the "
                             "iterative solution of MFEM equation systems.");
  params.addParam<MFEMFESpaceName>("fespace", "H(div) FESpace to use in HypreADS setup.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");

  return params;
}

MFEMHypreADS::MFEMHypreADS(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters),
    Moose::MFEM::LORInterface(parameters),
    _mfem_fespace(getMFEMProblem().getMFEMObject<MFEMFESpace>("MFEMFESpace",
                                                              getParam<MFEMFESpaceName>("fespace")))
{
  ConstructSolver();
}

void
MFEMHypreADS::SetSolverParameters(mfem::Solver & solver)
{
  auto & mfem_solver = static_cast<mfem::HypreADS &>(solver);
  mfem_solver.iterative_mode = getParam<bool>("use_initial_guess");
  mfem_solver.SetPrintLevel(getParam<int>("print_level"));
}

void
MFEMHypreADS::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreADS>(_mfem_fespace.getFESpace().get());
  SetSolverParameters(*solver);
  _solver = std::move(solver);
}

void
MFEMHypreADS::Update()
{
  Moose::MFEM::LinearSolverBase::Update();
  if (_lor)
  {
    if (_mfem_fespace.getFESpace()->GetMesh()->GetElement(0)->GetGeometryType() !=
        mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreADS Solver only supports hex meshes.");
    LORInterface::SetupLOR<mfem::HypreADS>(*this, *_equation_system);
  }
}

#endif

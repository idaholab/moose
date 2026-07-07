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
    Moose::MFEM::LORInterface(parameters),
    _mfem_fespace(getMFEMProblem().getMFEMObject<MFEMFESpace>("MFEMFESpace",
                                                              getParam<MFEMFESpaceName>("fespace")))
{
  ConstructSolver();
}

void
MFEMHypreAMS::SetSolverParameters(mfem::Solver & solver)
{
  auto & mfem_solver = static_cast<mfem::HypreAMS &>(solver);
  if (getParam<bool>("singular"))
    mfem_solver.SetSingularProblem();
  mfem_solver.iterative_mode = getParam<bool>("use_initial_guess");
  mfem_solver.SetPrintLevel(getParam<int>("print_level"));
}

void
MFEMHypreAMS::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreAMS>(_mfem_fespace.getFESpace().get());
  SetSolverParameters(*solver);
  _solver = std::move(solver);
}

void
MFEMHypreAMS::Update()
{
  Moose::MFEM::LinearSolverBase::Update();
  if (_lor)
  {
    if (_mfem_fespace.getFESpace()->GetMesh()->GetElement(0)->GetGeometryType() !=
        mfem::Geometry::Type::CUBE)
      mooseError("LOR HypreAMS Solver only supports hex meshes.");
    LORInterface::SetupLOR<mfem::HypreAMS>(*this, *_equation_system);
  }
}

#endif

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreGMRES.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreGMRES);

InputParameters
MFEMHypreGMRES::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("Hypre solver for the iterative solution of MFEM equation systems "
                             "using the generalized minimal residual method.");

  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<mfem::real_t>("l_abs_tol", 1e-50, "Set the absolute tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("kdim", 10, "Set the k-dimension.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<MFEMSolverName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMHypreGMRES::MFEMHypreGMRES(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters), Moose::MFEM::LORInterface(parameters)
{
  ConstructSolver();
}

void
MFEMHypreGMRES::SetSolverParameters(mfem::Solver & solver)
{
  auto & mfem_solver = static_cast<mfem::HypreGMRES &>(solver);
  mfem_solver.iterative_mode = getParam<bool>("use_initial_guess");
  mfem_solver.SetTol(getParam<mfem::real_t>("l_tol"));
  mfem_solver.SetAbsTol(getParam<mfem::real_t>("l_abs_tol"));
  mfem_solver.SetMaxIter(getParam<int>("l_max_its"));
  mfem_solver.SetKDim(getParam<int>("kdim"));
  mfem_solver.SetPrintLevel(getParam<int>("print_level"));
}

void
MFEMHypreGMRES::ConstructSolver()
{
  auto solver = std::make_unique<mfem::HypreGMRES>(getMFEMProblem().getComm());
  SetSolverParameters(*solver);
  SetPreconditioner(*solver);
  _solver = std::move(solver);
}

void
MFEMHypreGMRES::Update()
{
  Moose::MFEM::LinearSolverBase::Update();
  LORInterface::Update<mfem::HypreGMRES>(*this, *_equation_system);
}


#endif

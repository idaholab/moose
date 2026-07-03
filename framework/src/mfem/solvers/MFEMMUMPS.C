//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMMUMPS.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMMUMPS);

InputParameters
MFEMMUMPS::validParams()
{
  InputParameters params = Moose::MFEM::LinearSolverBase::validParams();
  params.addClassDescription("MFEM solver for performing direct solves of sparse systems in "
                             "parallel using the MUMPS library.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");

  return params;
}

MFEMMUMPS::MFEMMUMPS(const InputParameters & parameters)
  : Moose::MFEM::LinearSolverBase(parameters), Moose::MFEM::LORInterface(parameters)
{
  ConstructSolver();
}

void
MFEMMUMPS::ConstructSolver()
{
  auto solver = std::make_unique<mfem::MUMPSSolver>(getMFEMProblem().getComm());
  solver->iterative_mode = getParam<bool>("use_initial_guess");
  solver->SetPrintLevel(getParam<int>("print_level"));
  _solver = std::move(solver);
}

void
MFEMMUMPS::SetupLOR()
{
  if (_lor)
    mooseError("MUMPS solver does not support LOR solve");
}

#endif

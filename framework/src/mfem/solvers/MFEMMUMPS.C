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

registerMooseMFEMObject("MooseApp", MUMPS);

namespace Moose::MFEM
{
InputParameters
MUMPS::validParams()
{
  InputParameters params = LinearSolverBase::validParams();
  params.addClassDescription("MFEM solver for performing direct solves of sparse systems in "
                             "parallel using the MUMPS library.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");

  return params;
}

MUMPS::MUMPS(const InputParameters & parameters) : LinearSolverBase(parameters)
{
  constructSolver();
}

void
MUMPS::constructSolver()
{
  auto solver = std::make_unique<mfem::MUMPSSolver>(getMFEMProblem().getComm());
  solver->SetPrintLevel(getParam<int>("print_level"));
  _solver = std::move(solver);
}

void
MUMPS::setupLOR(mfem::ParBilinearForm &, mfem::Array<int> &)
{
  if (_lor)
    mooseError("MUMPS solver does not support LOR solve");
}

} // namespace Moose::MFEM
#endif

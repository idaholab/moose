//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSuperLU.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMSuperLU);

InputParameters
MFEMSuperLU::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("MFEM solver for performing direct solves of sparse systems in "
                             "parallel using the SuperLU_DIST library.");

  return params;
}

MFEMSuperLU::MFEMSuperLU(const InputParameters & parameters) : MFEMSolverBase(parameters)
{
  constructSolver();
}

void
MFEMSuperLU::constructSolver()
{
  auto solver = std::make_unique<Moose::MFEM::SuperLUSolver>(getMFEMProblem().getComm());
  solver->SetDeviceOffload(mfem::Device::IsAvailable());
  _solver = std::move(solver);
}

void
MFEMSuperLU::updateSolver(mfem::ParBilinearForm &, mfem::Array<int> &)
{
  if (_lor)
    mooseError("SuperLU solver does not support LOR solve");
}

#endif

#pragma once
#include "MFEMSuperLU.h"
#include "MFEMProblem.h"

registerMooseObject("PlatypusApp", MFEMSuperLU);

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
  constructSolver(parameters);
}

void
MFEMSuperLU::constructSolver(const InputParameters & parameters)
{
  _solver =
      std::make_shared<platypus::SuperLUSolver>(getMFEMProblem().mesh().getMFEMParMesh().GetComm());
}
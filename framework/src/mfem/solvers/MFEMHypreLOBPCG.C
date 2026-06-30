//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreLOBPCG.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreLOBPCG);

InputParameters
MFEMHypreLOBPCG::validParams()
{
  InputParameters params = Moose::MFEM::EigensolverBase::validParams();

  params.addClassDescription("Locally Optimal Block PCG eigensolver to iteratively compute the "
                             "lowest eigenmodes of a generalized eigenvalue problem.");
  params.addParam<int>("random_seed", 123, "Set the random seed for the solver.");

  return params;
}

MFEMHypreLOBPCG::MFEMHypreLOBPCG(const InputParameters & parameters)
  : Moose::MFEM::EigensolverBase(parameters)
{
  ConstructSolver();
}

void
MFEMHypreLOBPCG::ConstructSolver()
{
  _eigensolver = std::make_unique<mfem::HypreLOBPCG>(getMFEMProblem().getComm());

  _eigensolver->SetNumModes(numComputedModes());
  _eigensolver->SetRandomSeed(getParam<int>("random_seed"));
  _eigensolver->SetMaxIter(getParam<int>("l_max_its"));
  _eigensolver->SetTol(getParam<mfem::real_t>("l_tol"));
  _eigensolver->SetPrecondUsageMode(1);
  _eigensolver->SetPrintLevel(getParam<int>("print_level"));
  SetPreconditioner(*_eigensolver);
}

#endif

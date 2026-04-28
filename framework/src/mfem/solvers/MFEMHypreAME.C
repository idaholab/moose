//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMHypreAME.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMHypreAME);

InputParameters
MFEMHypreAME::validParams()
{
  InputParameters params = MFEMEigensolverBase::validParams();
  params.addClassDescription("Base class for defining MFEM eigensolver classes for Moose ");
  params.addParam<MFEMSolverName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMHypreAME::MFEMHypreAME(const InputParameters & parameters) : MFEMEigensolverBase(parameters)
{
  constructSolver();
}

void
MFEMHypreAME::constructSolver()
{
  _eigensolver = std::make_unique<mfem::HypreAME>(getMFEMProblem().getComm());

  _eigensolver->SetNumModes(_num_modes);
  _eigensolver->SetMaxIter(getParam<int>("l_max_its"));
  _eigensolver->SetTol(getParam<mfem::real_t>("l_tol"));
  _eigensolver->SetPrintLevel(getParam<int>("print_level"));
  setPreconditioner(*_eigensolver);
}

#endif

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
  InputParameters params = Moose::MFEM::EigensolverBase::validParams();
  params.addClassDescription("Hypre auxiliary-space Maxwell eigensolver to compute the lowest "
                             "eigenmodes of a generalized eigenvalue problem.");

  return params;
}

MFEMHypreAME::MFEMHypreAME(const InputParameters & parameters)
  : Moose::MFEM::EigensolverBase(parameters)
{
  ConstructSolver();
}

void
MFEMHypreAME::ConstructSolver()
{
  _eigensolver = std::make_unique<mfem::HypreAME>(getMFEMProblem().getComm());

  _eigensolver->SetNumModes(_num_modes);
  _eigensolver->SetMaxIter(getParam<int>("l_max_its"));
  _eigensolver->SetTol(getParam<mfem::real_t>("l_tol"));
  _eigensolver->SetPrintLevel(getParam<int>("print_level"));
  SetPreconditioner(*_eigensolver);
}

#endif

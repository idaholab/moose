//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMEigensolverBase.h"
#include "MFEMProblem.h"

InputParameters
MFEMEigensolverBase::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("Base class for defining MFEM eigensolver classes for Moose ");

  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<int>("random_seed", 123, "Set the random seed for the solver.");
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

MFEMEigensolverBase::MFEMEigensolverBase(const InputParameters & parameters)
  : MFEMSolverBase(parameters), _num_modes(getMFEMProblem().getParam<int>("num_modes"))
{
}

#endif

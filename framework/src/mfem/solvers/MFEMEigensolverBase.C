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

using MFEMHypreLOBPCG = MFEMEigensolverBase<mfem::HypreLOBPCG>;

registerMooseObject("MooseApp", MFEMHypreLOBPCG);

template <typename T>
InputParameters
MFEMEigensolverBase<T>::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("Base class for defining MFEM eigensolver classes for Moose ");

  params.addParam<mfem::real_t>("l_tol", 1e-5, "Set the relative tolerance.");
  params.addParam<int>("l_max_its", 10000, "Set the maximum number of iterations.");
  params.addParam<int>("print_level", 2, "Set the solver verbosity.");
  params.addParam<int>("random_seed", 123, "Set the random seed for the solver.");
  params.addParam<int>("num_modes", 1, "Set the number of lowest eigenmodes to compute.");
  params.addParam<UserObjectName>("preconditioner", "Optional choice of preconditioner to use.");

  return params;
}

template <typename T>
MFEMEigensolverBase<T>::MFEMEigensolverBase(const InputParameters & parameters) : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

template <typename T>
void
MFEMEigensolverBase<T>::constructSolver(const InputParameters &)
{
  _eigensolver = std::make_unique<T>(getMFEMProblem().getComm());
  _eigensolver->SetNumModes(getParam<int>("num_modes"));
  _eigensolver->SetRandomSeed(getParam<int>("random_seed"));
  _eigensolver->SetMaxIter(getParam<int>("l_max_its"));
  _eigensolver->SetTol(getParam<mfem::real_t>("l_tol"));
  _eigensolver->SetPrecondUsageMode(1);
  _eigensolver->SetPrintLevel(getParam<int>("print_level"));
  setPreconditioner(*_eigensolver);
}

template <typename T>
void
MFEMEigensolverBase<T>::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (_lor)
  mooseError("Eigensolver cannot use LOR method");

  if (_preconditioner)
  {
    _preconditioner->updateSolver(a, tdofs);
    setPreconditioner(*_eigensolver);
  }

  mfem::ParBilinearForm * m = new mfem::ParBilinearForm(a.ParFESpace());
  mfem::ConstantCoefficient one(1.0);
  m->AddDomainIntegrator(new mfem::MassIntegrator(one));
  m->Assemble();

  // Shift the eigenvalue corresponding to eliminated dofs to a large value
  m->EliminateEssentialBCDiag(tdofs, std::numeric_limits<mfem::real_t>::min());
  m->Finalize();
  _M.reset(m->ParallelAssemble());
  _eigensolver->SetMassMatrix(*_M);

}

template class MFEMEigensolverBase<mfem::HypreLOBPCG>;

#endif

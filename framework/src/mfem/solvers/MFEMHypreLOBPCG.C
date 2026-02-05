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
  InputParameters params = MFEMEigensolverBase::validParams();
  params.addClassDescription("Base class for defining MFEM eigensolver classes for Moose ");
  params.addRequiredParam<MFEMScalarCoefficientName>(
      "coefficient", "Name of the scalar coefficient for the mass matrix.");

  return params;
}

MFEMHypreLOBPCG::MFEMHypreLOBPCG(const InputParameters & parameters)
  : MFEMEigensolverBase(parameters), _coef(getScalarCoefficient("coefficient"))
{
  constructSolver(parameters);
}

void
MFEMHypreLOBPCG::constructSolver(const InputParameters &)
{
  _eigensolver = std::make_unique<mfem::HypreLOBPCG>(getMFEMProblem().getComm());

  _eigensolver->SetNumModes(_num_modes);
  _eigensolver->SetRandomSeed(getParam<int>("random_seed"));
  _eigensolver->SetMaxIter(getParam<int>("l_max_its"));
  _eigensolver->SetTol(getParam<mfem::real_t>("l_tol"));
  _eigensolver->SetPrecondUsageMode(1);
  _eigensolver->SetPrintLevel(getParam<int>("print_level"));

}

void
MFEMHypreLOBPCG::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (_lor)
    mooseError("Eigensolver cannot use LOR method");

  if (_preconditioner)
  {
    _preconditioner->updateSolver(a, tdofs);
    setPreconditioner(static_cast<mfem::HypreLOBPCG &>(*_eigensolver));
  }

}

#endif

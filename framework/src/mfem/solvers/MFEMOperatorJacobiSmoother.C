//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMOperatorJacobiSmoother.h"
#include "MFEMProblem.h"

registerMooseObject("MooseApp", MFEMOperatorJacobiSmoother);

InputParameters
MFEMOperatorJacobiSmoother::validParams()
{
  InputParameters params = MFEMSolverBase::validParams();
  params.addClassDescription("MFEM solver for performing Jacobi smoothing of the equation system.");

  return params;
}

MFEMOperatorJacobiSmoother::MFEMOperatorJacobiSmoother(const InputParameters & parameters)
  : MFEMSolverBase(parameters)
{
  constructSolver(parameters);
}

void
MFEMOperatorJacobiSmoother::constructSolver(const InputParameters &)
{
  _solver = std::make_unique<mfem::OperatorJacobiSmoother>();
}

void
MFEMOperatorJacobiSmoother::updateSolver(mfem::ParBilinearForm & a, mfem::Array<int> & tdofs)
{
  if (getParam<bool>("low_order_refined"))
    _solver.reset(new mfem::LORSolver<mfem::OperatorJacobiSmoother>(a, tdofs));
}

#endif

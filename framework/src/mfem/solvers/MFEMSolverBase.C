//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMSolverBase.h"
#include "MFEMProblem.h"

namespace Moose::MFEM
{
InputParameters
SolverBase::validParams()
{
  InputParameters params = MFEMObject::validParams();
  params.addClassDescription("Base class for defining mfem::Solver derived classes for Moose.");
  params.registerBase("Moose::MFEM::SolverBase");
  params.registerSystemAttributeName("Moose::MFEM::SolverBase");
  params.addParam<bool>("use_initial_guess",
                        false,
                        "Whether to preserve the current MFEM solution vector as the initial "
                        "guess for an iterative solver.");
  return params;
}

SolverBase::SolverBase(const InputParameters & parameters)
  : MFEMObject(parameters), _solver{nullptr}
{
}

void
SolverBase::SetOperator(mfem::Operator & op)
{
  UpdateEquationSystemContext();
  SetOperatorImpl(op);
}
} // namespace Moose::MFEM

#endif

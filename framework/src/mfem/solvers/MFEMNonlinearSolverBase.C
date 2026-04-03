//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMNonlinearSolverBase.h"

namespace Moose::MFEM
{
InputParameters
NonlinearSolverBase::validParams()
{
  InputParameters params = SolverBase::validParams();
  params.addClassDescription("Base class for defining nonlinear MFEM solver strategies for Moose.");
  params.addParam<unsigned int>("max_its", 1, "Maximum nonlinear iterations.");
  params.addParam<Real>("abs_tol", 1.0e-50, "Absolute nonlinear tolerance.");
  params.addParam<Real>("rel_tol", 1.0e-8, "Relative nonlinear tolerance.");
  params.addParam<unsigned int>("print_level", 1, "Solver verbosity.");
  params.addParam<bool>("use_initial_guess",
                        true,
                        "Whether to preserve the current MFEM solution vector as the initial "
                        "guess for the nonlinear solver.");
  return params;
}

NonlinearSolverBase::NonlinearSolverBase(const InputParameters & parameters)
  : SolverBase(parameters)
{
}
} // namespace Moose::MFEM

#endif

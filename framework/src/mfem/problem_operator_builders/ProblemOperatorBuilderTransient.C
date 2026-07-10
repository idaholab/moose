//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "ProblemOperatorBuilderTransient.h"
#include "MFEMProblem.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{
registerMooseObject("MooseApp", ProblemOperatorBuilderTransient);
};

InputParameters
Moose::MFEM::ProblemOperatorBuilderTransient::validParams()
{
  InputParameters params = ProblemOperatorBuilderBase::validParams();
  return params;
};

Moose::MFEM::ProblemOperatorBuilderTransient::ProblemOperatorBuilderTransient(
    const InputParameters & parameters)
  : ProblemOperatorBuilderBase(parameters) {};

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
Moose::MFEM::ProblemOperatorBuilderTransient::createProblemOperator(MFEMProblem & mfemProb)
{
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase> probOp;

  // Construct the problem operator
  mfemProb.getProblemData().eqn_system = std::make_shared<Moose::MFEM::TimeDependentEquationSystem>(
      mfemProb.getProblemData().time_derivative_map);
  probOp = std::make_shared<Moose::MFEM::TimeDependentEquationSystemProblemOperator>(mfemProb);
  return probOp;
};

#endif

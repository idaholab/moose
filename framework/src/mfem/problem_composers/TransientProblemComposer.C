//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "TransientProblemComposer.h"
#include "MFEMProblem.h"
#include "ProblemOperatorBase.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "TimeDependentEquationSystem.h"
#include "TimeDependentEquationSystemProblemOperator.h"

namespace Moose::MFEM
{
registerMooseObject("MooseApp", TransientProblemComposer);
}

InputParameters
Moose::MFEM::TransientProblemComposer::validParams()
{
  InputParameters params = ProblemComposerBase::validParams();
  return params;
}

Moose::MFEM::TransientProblemComposer::TransientProblemComposer(const InputParameters & parameters)
  : ProblemComposerBase(parameters)
{
}

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
Moose::MFEM::TransientProblemComposer::createProblemOperator(MFEMProblem & mfem_problem)
{
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase> _problem_operator;

  // Construct the problem operator
  mfem_problem.getProblemData().eqn_system =
      std::make_shared<Moose::MFEM::TimeDependentEquationSystem>(
          mfem_problem.getProblemData().time_derivative_map);
  _problem_operator =
      std::make_shared<Moose::MFEM::TimeDependentEquationSystemProblemOperator>(mfem_problem);
  return _problem_operator;
}

#endif

//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMWeakFormComposer.h"
#include "MFEMProblem.h"
#include "ProblemOperatorBase.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "EquationSystem.h"
#include "EquationSystemProblemOperator.h"

namespace Moose::MFEM
{
registerMooseObject("MooseApp", MFEMWeakFormComposer);
}

InputParameters
Moose::MFEM::MFEMWeakFormComposer::validParams()
{
  InputParameters params = MFEMProblemComposer::validParams();
  return params;
}

Moose::MFEM::MFEMWeakFormComposer::MFEMWeakFormComposer(const InputParameters & parameters)
  : MFEMProblemComposer(parameters)
{
}

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
Moose::MFEM::MFEMWeakFormComposer::createProblemOperator(MFEMProblem & mfem_problem)
{
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase> _problem_operator;

  // Construct a standard problem operator
  if (mfem_problem.getNumericType() == MFEMProblem::NumericType::REAL)
  {
    mfem_problem.getProblemData().eqn_system = std::make_shared<Moose::MFEM::EquationSystem>();
    _problem_operator = std::make_shared<Moose::MFEM::EquationSystemProblemOperator>(mfem_problem);
  }
  else
  {
    mooseError("Wrong numeric type. "
               "Please set the Problem numeric type to 'real'.");
  }
  return _problem_operator;
}

#endif

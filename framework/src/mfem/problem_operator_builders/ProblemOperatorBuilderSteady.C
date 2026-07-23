//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "ProblemOperatorBuilderSteady.h"
#include "MFEMProblem.h"
#include "ProblemOperatorBase.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "EquationSystem.h"
#include "ComplexEquationSystem.h"
#include "EigenproblemEquationSystem.h"
#include "EquationSystemProblemOperator.h"
#include "EigenproblemESProblemOperator.h"
#include "ComplexEquationSystemProblemOperator.h"

namespace Moose::MFEM
{
registerMooseObject("MooseApp", ProblemOperatorBuilderSteady);
}

InputParameters
Moose::MFEM::ProblemOperatorBuilderSteady::validParams()
{
  InputParameters params = ProblemOperatorBuilderBase::validParams();
  return params;
}

Moose::MFEM::ProblemOperatorBuilderSteady::ProblemOperatorBuilderSteady(
    const InputParameters & parameters)
  : ProblemOperatorBuilderBase(parameters)
{
}

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
Moose::MFEM::ProblemOperatorBuilderSteady::createProblemOperator(MFEMProblem & mfem_problem)
{
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase> _problem_operator;

  // Construct a standard problem operator
  if (mfem_problem.getNumericType() == MFEMProblem::NumericType::REAL)
  {
    if (dynamic_cast<MFEMEigenproblem *>(&mfem_problem))
    {
      mfem_problem.getProblemData().eqn_system =
          std::make_shared<Moose::MFEM::EigenproblemEquationSystem>();
      _problem_operator =
          std::make_shared<Moose::MFEM::EigenproblemESProblemOperator>(mfem_problem);
    }
    else
    {
      mfem_problem.getProblemData().eqn_system = std::make_shared<Moose::MFEM::EquationSystem>();
      _problem_operator =
          std::make_shared<Moose::MFEM::EquationSystemProblemOperator>(mfem_problem);
    }
  }
  else if (mfem_problem.getNumericType() == MFEMProblem::NumericType::COMPLEX)
  {
    mfem_problem.getProblemData().eqn_system =
        std::make_shared<Moose::MFEM::ComplexEquationSystem>();
    _problem_operator =
        std::make_shared<Moose::MFEM::ComplexEquationSystemProblemOperator>(mfem_problem);
  }
  else
  {
    mooseError("Unknown numeric type. "
               "Please set the Problem numeric type to either 'real' or 'complex'.");
  }
  return _problem_operator;
}

#endif

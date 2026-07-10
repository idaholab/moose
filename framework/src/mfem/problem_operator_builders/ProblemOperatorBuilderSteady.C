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
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"

namespace Moose::MFEM
{
registerMooseObject("MooseApp", ProblemOperatorBuilderSteady);
};

InputParameters
Moose::MFEM::ProblemOperatorBuilderSteady::validParams()
{
  InputParameters params = ProblemOperatorBuilderBase::validParams();
  return params;
};

Moose::MFEM::ProblemOperatorBuilderSteady::ProblemOperatorBuilderSteady(
    const InputParameters & parameters)
  : ProblemOperatorBuilderBase(parameters) {};

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
Moose::MFEM::ProblemOperatorBuilderSteady::createProblemOperator(MFEMProblem & mfemProb)
{
  std::shared_ptr<Moose::MFEM::ProblemOperatorBase> probOp;

  // Construct a standard problem operator
  if (mfemProb.getNumericType() == MFEMProblem::NumericType::REAL)
  {
    if (dynamic_cast<MFEMEigenproblem *>(&mfemProb))
    {
      mfemProb.getProblemData().eqn_system =
          std::make_shared<Moose::MFEM::EigenproblemEquationSystem>();
      probOp = std::make_shared<Moose::MFEM::EigenproblemESProblemOperator>(mfemProb);
    }
    else
    {
      mfemProb.getProblemData().eqn_system = std::make_shared<Moose::MFEM::EquationSystem>();
      probOp = std::make_shared<Moose::MFEM::EquationSystemProblemOperator>(mfemProb);
    }
  }
  else if (mfemProb.getNumericType() == MFEMProblem::NumericType::COMPLEX)
  {
    mfemProb.getProblemData().eqn_system = std::make_shared<Moose::MFEM::ComplexEquationSystem>();
    probOp = std::make_shared<Moose::MFEM::ComplexEquationSystemProblemOperator>(mfemProb);
  }
  else
  {
    mooseError("Unknown numeric type. "
               "Please set the Problem numeric type to either 'real' or 'complex'.");
  }
  return probOp;
};

#endif

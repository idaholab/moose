//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#include "MFEMEigenWeakFormProblemComposer.h"
#include "MFEMProblem.h"
#include "ProblemOperatorBase.h"
#include "libmesh/ignore_warnings.h"
#include "mfem/miniapps/common/mesh_extras.hpp"
#include "libmesh/restore_warnings.h"
#include "EigenproblemEquationSystem.h"
#include "EigenproblemESProblemOperator.h"

namespace Moose::MFEM
{
registerMooseObject("MooseApp", MFEMEigenWeakFormProblemComposer);
}

std::shared_ptr<Moose::MFEM::ProblemOperatorBase>
Moose::MFEM::MFEMEigenWeakFormProblemComposer::createProblemOperator(MFEMProblem & mfem_problem)
{
  // Construct a standard problem operator
  if (mfem_problem.getNumericType() != MFEMProblem::NumericType::REAL)
    mooseError("Wrong numeric type. Please set the Problem numeric type to 'real'.");

  if (!dynamic_cast<MFEMEigenproblem *>(&mfem_problem))
    mooseError("Not an eigen value problem. ");

  mfem_problem.getProblemData().eqn_system =
      std::make_shared<Moose::MFEM::EigenproblemEquationSystem>();

  return std::make_shared<Moose::MFEM::EigenproblemESProblemOperator>(mfem_problem);
}

#endif

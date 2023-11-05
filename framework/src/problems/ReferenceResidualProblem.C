//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReferenceResidualProblem.h"
#include "ReferenceResidualConvergence.h"

registerMooseObject("MooseApp", ReferenceResidualProblem);

InputParameters
ReferenceResidualProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params += ReferenceResidualConvergence::validCommonReferenceResidualProblemParams();
  params += ReferenceConvergenceInterface::validParams();
  params.addClassDescription("Problem that checks for convergence relative to "
                             "a user-supplied reference quantity rather than "
                             "the initial residual");

  return params;
}

ReferenceResidualProblem::ReferenceResidualProblem(const InputParameters & params)
  : FEProblem(params), ReferenceConvergenceInterface(this)
{
}

void
ReferenceResidualProblem::addDefaultConvergence()
{
  const std::string class_name = "ReferenceResidualConvergence";
  InputParameters params = _factory.getValidParams(class_name);
  params.applyParameters(parameters());
  setNonlinearConvergenceName("reference_residual");
  addConvergence(class_name, _nonlinear_convergence_name, params);
}

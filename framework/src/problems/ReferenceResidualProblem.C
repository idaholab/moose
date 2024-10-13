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

using namespace libMesh;

registerMooseObject("MooseApp", ReferenceResidualProblem);

InputParameters
ReferenceResidualProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params += ReferenceResidualInterface::validParams();

  params.addClassDescription("Problem that checks for convergence relative to "
                             "a user-supplied reference quantity rather than "
                             "the initial residual");

  return params;
}

ReferenceResidualProblem::ReferenceResidualProblem(const InputParameters & params)
  : FEProblem(params), ReferenceResidualInterface(this)
{
}

void
ReferenceResidualProblem::addDefaultNonlinearConvergence(const InputParameters & params_to_apply)
{
  const std::string class_name = "ReferenceResidualConvergence";
  InputParameters params = _factory.getValidParams(class_name);
  params.applyParameters(params_to_apply);
  params.applyParameters(parameters());
  params.set<bool>("added_as_default") = true;
  for (const auto & conv_name : getNonlinearConvergenceNames())
    addConvergence(class_name, conv_name, params);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReferenceResidualInterface.h"
#include "MooseObject.h"
#include "MooseEnum.h"

InputParameters
ReferenceResidualInterface::validParams()
{
  InputParameters params = emptyInputParameters();

  params.addParam<std::vector<NonlinearVariableName>>(
      "solution_variables", "Set of solution variables to be checked for relative convergence");
  params.addParam<std::vector<AuxVariableName>>(
      "reference_residual_variables",
      "Set of variables that provide reference residuals for relative convergence check");
  params.addParam<TagName>("reference_vector", "The tag name of the reference residual vector.");
  params.addParam<Real>("acceptable_multiplier",
                        1.0,
                        "Multiplier applied to relative tolerance for acceptable limit");
  params.addParam<unsigned int>(
      "acceptable_iterations",
      0,
      "Iterations after which convergence to acceptable limits is accepted");
  params.addParam<std::vector<std::vector<NonlinearVariableName>>>(
      "group_variables",
      "Name of variables that are grouped together to check convergence. (Multiple groups can be "
      "provided, separated by semicolon)");
  params.addParam<std::vector<NonlinearVariableName>>(
      "converge_on",
      {},
      "If supplied, use only these variables in the individual variable convergence check");
  MooseEnum Lnorm("global_L2 local_L2 global_Linf local_Linf", "global_L2");
  params.addParam<MooseEnum>(
      "normalization_type",
      Lnorm,
      "The normalization type used to compare the reference and actual residuals.");
  Lnorm.addDocumentation("global_L2",
                         "Compare the L2 norm of the residual vector to the L2 norm of the "
                         "absolute reference vector to determine relative convergence");
  Lnorm.addDocumentation(
      "local_L2",
      "Compute the L2 norm of the residual vector divided component-wise by the absolute reference "
      "vector to the L2 norm of the absolute reference vector to determine relative convergence");
  Lnorm.addDocumentation(
      "global_Linf",
      "Compare the L-infinity norm of the residual vector to the L-infinity norm of the "
      "absolute reference vector to determine relative convergence");
  Lnorm.addDocumentation(
      "local_Linf",
      "Compute the L-infinity norm of the residual vector divided component-wise "
      "by the absolute reference "
      "vector to the L-infinity norm of the absolute reference vector to "
      "determine relative convergence");

  MooseEnum zero_ref_res("zero_tolerance relative_tolerance", "relative_tolerance");
  params.addParam<MooseEnum>("zero_reference_residual_treatment",
                             zero_ref_res,
                             "Determine behavior if a reference residual value of zero is present "
                             "for a particular variable.");
  zero_ref_res.addDocumentation("zero_tolerance",
                                "Solve is treated as converged if the residual is zero");
  zero_ref_res.addDocumentation(
      "relative_tolerance",
      "Solve is treated as converged if the residual is below the relative tolerance");

  params.addParamNamesToGroup("acceptable_iterations acceptable_multiplier",
                              "Acceptable convergence");
  params.addParamNamesToGroup("reference_vector reference_residual_variables",
                              "Reference residual");
  params.addParamNamesToGroup("solution_variables group_variables",
                              "Variables to check for convergence");

  return params;
}

ReferenceResidualInterface::ReferenceResidualInterface(const MooseObject * moose_object)
  : _use_group_variables(false)
{
  if (moose_object->isParamValid("group_variables"))
  {
    _group_variables =
        moose_object->getParam<std::vector<std::vector<NonlinearVariableName>>>("group_variables");
    _use_group_variables = true;
  }
}

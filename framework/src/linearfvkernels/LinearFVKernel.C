//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LinearFVKernel.h"
#include "Assembly.h"
#include "SubProblem.h"

InputParameters
LinearFVKernel::validParams()
{
  InputParameters params = LinearSystemContributionObject::validParams();
  params += BlockRestrictable::validParams();
  params += NonADFunctorInterface::validParams();
  params += FVRelationshipManagerInterface::validParams();

  params.registerBase("LinearFVKernel");
  return params;
}

LinearFVKernel::LinearFVKernel(const InputParameters & params)
  : LinearSystemContributionObject(params),
    BlockRestrictable(this),
    NonADFunctorInterface(this),
    MooseVariableInterface(this,
                           false,
                           "variable",
                           Moose::VarKindType::VAR_SOLVER,
                           Moose::VarFieldType::VAR_FIELD_STANDARD),
    MooseVariableDependencyInterface(this),
    _var(*mooseLinearVariableFV()),
    _var_num(_var.number()),
    _sys_num(_sys.number())
{
  addMooseVariableDependency(&_var);
}

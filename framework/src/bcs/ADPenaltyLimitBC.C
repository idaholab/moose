//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPenaltyLimitBC.h"

registerMooseObject("MooseApp", ADPenaltyLimitBC);

InputParameters
ADPenaltyLimitBC::validParams()
{
  InputParameters params = ADPenaltyDirichletBC::validParams();
  params.addClassDescription(
      "Enforces a Dirichlet boundary condition in a weak sense by penalizing differences between "
      "the current solution and the Dirichlet data if the variable value is above or below the "
      "limit.");
  MooseEnum apply_penalty_when("greaterthan lessthan", "greaterthan");
  params.addParam<MooseEnum>(
      "apply_penalty_when",
      apply_penalty_when,
      "Apply the penalty when the variable value is either greaterthan or lessthan the value");
  return params;
}

ADPenaltyLimitBC::ADPenaltyLimitBC(const InputParameters & parameters)
  : ADPenaltyDirichletBC(parameters),
    _apply_penalty_when(
        getParam<MooseEnum>("apply_penalty_when").template getEnum<ApplyPenaltyWhen>())
{
}

ADReal
ADPenaltyLimitBC::computeQpResidual()
{
  if ((_apply_penalty_when == ApplyPenaltyWhen::GREATERTHAN && _u[_qp] > _v) ||
      (_apply_penalty_when == ApplyPenaltyWhen::LESSTHAN && _u[_qp] < _v))
    return ADPenaltyDirichletBC::computeQpResidual();
  else
    return 0.0;
}

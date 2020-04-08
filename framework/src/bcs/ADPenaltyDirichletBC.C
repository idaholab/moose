//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPenaltyDirichletBC.h"

registerADMooseObject("MooseApp", ADPenaltyDirichletBC);

defineADLegacyParams(ADPenaltyDirichletBC);

template <ComputeStage compute_stage>
InputParameters
ADPenaltyDirichletBC<compute_stage>::validParams()
{
  InputParameters params = ADIntegratedBC<compute_stage>::validParams();
  params.addRequiredParam<Real>("penalty", "Penalty scalar");
  params.addParam<Real>("value", 0.0, "Boundary value of the variable");
  params.declareControllable("value");
  params.addClassDescription("Enforces a Dirichlet boundary condition "
                             "in a weak sense by penalizing differences between the current "
                             "solution and the Dirichlet data.");
  return params;
}

template <ComputeStage compute_stage>
ADPenaltyDirichletBC<compute_stage>::ADPenaltyDirichletBC(const InputParameters & parameters)
  : ADIntegratedBC<compute_stage>(parameters),
    _p(getParam<Real>("penalty")),
    _v(getParam<Real>("value"))
{
}

template <ComputeStage compute_stage>
ADReal
ADPenaltyDirichletBC<compute_stage>::computeQpResidual()
{
  return _p * _test[_i][_qp] * (-_v + _u[_qp]);
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "LevelSetOlssonReinitialization.h"

registerMooseObject("LevelSetApp", LevelSetOlssonReinitialization);

InputParameters
LevelSetOlssonReinitialization::validParams()
{
  InputParameters params = ADKernelGrad::validParams();
  params.addClassDescription("The re-initialization equation defined by Olsson et. al. (2007).");
  params.addRequiredCoupledVar(
      "phi_0", "The level set variable to be reinitialized as signed distance function.");
  params.addParam<bool>(
      "use_modified_reinitilization_formulation",
      false,
      "Use the modified reinitilization formulation (Olsson et. al. (2007), section 2.2.1).");
  params.addRequiredParam<PostprocessorName>(
      "epsilon", "The epsilon coefficient to be used in the reinitialization calculation.");
  return params;
}

LevelSetOlssonReinitialization::LevelSetOlssonReinitialization(const InputParameters & parameters)
  : ADKernelGrad(parameters),
    _grad_levelset_0(adCoupledGradient("phi_0")),
    _epsilon(getPostprocessorValue("epsilon")),
    _use_modified_reinitilization_formulation(
        getParam<bool>("use_modified_reinitilization_formulation"))
{
}

ADRealVectorValue
LevelSetOlssonReinitialization::precomputeQpResidual()
{
  ADReal s = _grad_levelset_0[_qp].norm() + std::numeric_limits<ADReal>::epsilon();
  ADRealVectorValue n_hat = _grad_levelset_0[_qp] / s;
  ADRealVectorValue f = _u[_qp] * (1 - _u[_qp]) * n_hat;
  if (_use_modified_reinitilization_formulation)
    return (-f + _epsilon * (_grad_u[_qp] * n_hat) * n_hat);
  else
    return (-f + _epsilon * _grad_u[_qp]);
}

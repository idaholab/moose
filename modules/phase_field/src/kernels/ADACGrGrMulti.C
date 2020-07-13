//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADACGrGrMulti.h"

registerMooseObject("PhaseFieldApp", ADACGrGrMulti);

InputParameters
ADACGrGrMulti::validParams()
{
  InputParameters params = ADGrainGrowthBase::validParams();
  params.addClassDescription("Multi-phase poly-crystalline Allen-Cahn Kernel");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "gamma_names",
      "List of gamma material property names for each other order parameter. Place "
      "in same order as order parameters (v)!");
  return params;
}

ADACGrGrMulti::ADACGrGrMulti(const InputParameters & parameters)
  : ADGrainGrowthBase(parameters),
    _gamma_names(getParam<std::vector<MaterialPropertyName>>("gamma_names")),
    _num_j(_gamma_names.size()),
    _prop_gammas(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != coupledComponents("v"))
    paramError("gamma_names",
               "Need to pass in as many gamma_names as coupled variables in v in ADACGrGrMulti");

  for (unsigned int n = 0; n < _num_j; ++n)
    _prop_gammas[n] = &getADMaterialProperty<Real>(_gamma_names[n]);
}

ADReal
ADACGrGrMulti::computeDFDOP()
{
  // Sum all other order parameters
  ADReal SumGammaEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumGammaEtaj += (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp];

  return _mu[_qp] * (_u[_qp] * _u[_qp] * _u[_qp] - _u[_qp] + 2.0 * _u[_qp] * SumGammaEtaj);
}

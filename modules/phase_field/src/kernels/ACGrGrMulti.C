//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ACGrGrMulti.h"

registerMooseObject("PhaseFieldApp", ACGrGrMulti);

InputParameters
ACGrGrMulti::validParams()
{
  InputParameters params = ACGrGrBase::validParams();
  params.addClassDescription("Multi-phase poly-crystalline Allen-Cahn Kernel");
  params.addRequiredParam<std::vector<MaterialPropertyName>>(
      "gamma_names",
      "List of gamma material property names for each other order parameter. Place "
      "in same order as order parameters (v)!");
  return params;
}

ACGrGrMulti::ACGrGrMulti(const InputParameters & parameters)
  : ACGrGrBase(parameters),
    _gamma_names(getParam<std::vector<MaterialPropertyName>>("gamma_names")),
    _num_j(_gamma_names.size()),
    _prop_gammas(_num_j),
    _uname(getParam<NonlinearVariableName>("variable")),
    _dmudu(getMaterialPropertyDerivative<Real>("mu", _uname)),
    _vname(getParam<std::vector<VariableName>>("v")),
    _dmudEtaj(_num_j)
{
  // check passed in parameter vectors
  if (_num_j != coupledComponents("v"))
    paramError("gamma_names",
               "Need to pass in as many gamma_names as coupled variables in v in ACGrGrMulti");

  for (unsigned int n = 0; n < _num_j; ++n)
  {
    _prop_gammas[n] = &getMaterialPropertyByName<Real>(_gamma_names[n]);
    _dmudEtaj[n] = &getMaterialPropertyDerivative<Real>("mu", _vname[n]);
  }
}

Real
ACGrGrMulti::computeDFDOP(PFFunctionType type)
{
  // Sum all other order parameters
  Real SumGammaEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumGammaEtaj += (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp];

  // Calculate either the residual or Jacobian of the grain growth free energy
  switch (type)
  {
    case Residual:
    {
      return _mu[_qp] * computedF0du();
    }

    case Jacobian:
    {
      Real d2f0du2 = 3.0 * _u[_qp] * _u[_qp] - 1.0 + 2.0 * SumGammaEtaj;
      return _phi[_j][_qp] * (_mu[_qp] * d2f0du2 + _dmudu[_qp] * computedF0du());
    }

    default:
      mooseError("Invalid type passed in");
  }
}

Real
ACGrGrMulti::computeQpOffDiagJacobian(unsigned int jvar)
{
  for (unsigned int i = 0; i < _op_num; ++i)
    if (jvar == _vals_var[i])
    {
      // Derivative of SumGammaEtaj
      const Real dSumGammaEtaj = 2.0 * (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp];
      const Real dDFDOP = _mu[_qp] * 2.0 * _u[_qp] * dSumGammaEtaj;

      return _L[_qp] * _test[_i][_qp] * _phi[_j][_qp] *
             (dDFDOP + (*_dmudEtaj[i])[_qp] * computedF0du());
    }

  return 0.0;
}

Real
ACGrGrMulti::computedF0du()
{
  Real SumGammaEtaj = 0.0;
  for (unsigned int i = 0; i < _op_num; ++i)
    SumGammaEtaj += (*_prop_gammas[i])[_qp] * (*_vals[i])[_qp] * (*_vals[i])[_qp];

  return _u[_qp] * _u[_qp] * _u[_qp] - _u[_qp] + 2.0 * _u[_qp] * SumGammaEtaj;
}

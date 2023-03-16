//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ForceDensityMaterial.h"

registerMooseObject("PhaseFieldApp", ForceDensityMaterial);

InputParameters
ForceDensityMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Calculating the force density acting on a grain");
  params.addCoupledVar("etas", "Array of coupled order parameters");
  params.addCoupledVar("c", "Concentration field");
  params.addParam<Real>("ceq", 0.9816, "Equilibrium density");
  params.addParam<Real>("cgb", 0.25, "Threshold Concentration for GB");
  params.addParam<Real>("k", 100.0, "stiffness constant");
  return params;
}

ForceDensityMaterial::ForceDensityMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _c(coupledValue("c")),
    _c_name(coupledName("c", 0)),
    _ceq(getParam<Real>("ceq")),
    _cgb(getParam<Real>("cgb")),
    _k(getParam<Real>("k")),
    _op_num(coupledComponents(
        "etas")), // determine number of grains from the number of names passed in.
    _vals(coupledValues("etas")),
    _grad_vals(coupledGradients("etas")),
    _vals_name(coupledNames("etas")),
    _product_etas(_op_num),
    _sum_grad_etas(_op_num),
    _dF(declareProperty<std::vector<RealGradient>>("force_density")),
    _dFdc(declarePropertyDerivative<std::vector<RealGradient>>("force_density", _c_name)),
    _dFdgradeta(_op_num)
{
  // Loop through grains and load derivatives
  for (unsigned int i = 0; i < _op_num; ++i)
    if (!isCoupledConstant(_vals_name[i]))
      _dFdgradeta[i] =
          &declarePropertyDerivative<std::vector<Real>>("force_density", _vals_name[i]);
}

void
ForceDensityMaterial::computeQpProperties()
{
  _dF[_qp].resize(_op_num);
  _dFdc[_qp].resize(_op_num);

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _sum_grad_etas[i] = 0.0;
    for (unsigned int j = 0; j < _op_num; ++j)
      if (j != i)
      {
        _product_etas[i] = (*_vals[i])[_qp] * (*_vals[j])[_qp] >= _cgb ? 1.0 : 0.0;
        _sum_grad_etas[i] += _product_etas[i] * ((*_grad_vals[i])[_qp] - (*_grad_vals[j])[_qp]);
      }
    _dF[_qp][i] = _k * (_c[_qp] - _ceq) * _sum_grad_etas[i];
    _dFdc[_qp][i] = _k * _sum_grad_etas[i];
  }

  for (unsigned int i = 0; i < _op_num; ++i)
  {
    if (_dFdgradeta[i])
      (*_dFdgradeta[i])[_qp].resize(_op_num);
    for (unsigned int j = 0; j < _op_num; ++j)
    {
      for (unsigned int k = 0; k < _op_num; ++k)
        if (k != j)
          _product_etas[j] = (*_vals[j])[_qp] * (*_vals[k])[_qp] >= _cgb ? 1.0 : 0.0;

      if (_dFdgradeta[i])
      {
        if (j == i)
          (*_dFdgradeta[i])[_qp][j] = _k * _product_etas[j] * (_c[_qp] - _ceq);
        else
          (*_dFdgradeta[i])[_qp][j] = -_k * _product_etas[j] * (_c[_qp] - _ceq);
      }
    }
  }
}

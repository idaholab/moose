/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ForceDensityMaterial.h"

template <>
InputParameters
validParams<ForceDensityMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription("Calculating the force density acting on a grain");
  params.addCoupledVar("etas", "Array of coupled order parameters");
  params.addCoupledVar("c", "Concentration field");
  params.addParam<Real>("ceq", 0.9816, "Equilibrium density");
  params.addParam<Real>("cgb", 0.25, "Thresold Concentration for GB");
  params.addParam<Real>("k", 100.0, "stiffness constant");
  return params;
}

ForceDensityMaterial::ForceDensityMaterial(const InputParameters & parameters)
  : DerivativeMaterialInterface<Material>(parameters),
    _c(coupledValue("c")),
    _c_name(getVar("c", 0)->name()),
    _ceq(getParam<Real>("ceq")),
    _cgb(getParam<Real>("cgb")),
    _k(getParam<Real>("k")),
    _op_num(coupledComponents(
        "etas")),   // determine number of grains from the number of names passed in.
    _vals(_op_num), // Size variable arrays
    _grad_vals(_op_num),
    _vals_name(_op_num),
    _product_etas(_op_num),
    _sum_grad_etas(_op_num),
    _dF(declareProperty<std::vector<RealGradient>>("force_density")),
    _dFdc(declarePropertyDerivative<std::vector<RealGradient>>("force_density", _c_name)),
    _dFdgradeta(_op_num)
{
  // Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _op_num; ++i)
  {
    _vals[i] = &coupledValue("etas", i);
    _grad_vals[i] = &coupledGradient("etas", i);
    _vals_name[i] = getVar("etas", i)->name();
    _dFdgradeta[i] = &declarePropertyDerivative<std::vector<Real>>("force_density", _vals_name[i]);
  }
}

void
ForceDensityMaterial::computeQpProperties()
{
  _dF[_qp].resize(_op_num);
  _dFdc[_qp].resize(_op_num);

  Real c = _c[_qp];
  if (c >= _ceq)
    c = _ceq;
  else if (c < 0.0)
    c = 0.0;

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
    (*_dFdgradeta[i])[_qp].resize(_op_num);
    for (unsigned int j = 0; j < _op_num; ++j)
    {
      for (unsigned int k = 0; k < _op_num; ++k)
        if (k != j)
          _product_etas[j] = (*_vals[j])[_qp] * (*_vals[k])[_qp] >= _cgb ? 1.0 : 0.0;

      if (j == i)
        (*_dFdgradeta[i])[_qp][j] = _k * _product_etas[j] * (_c[_qp] - _ceq);
      else
        (*_dFdgradeta[i])[_qp][j] = -_k * _product_etas[j] * (_c[_qp] - _ceq);
    }
  }
}

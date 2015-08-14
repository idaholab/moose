/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ForceDensityMaterial.h"

template<>
InputParameters validParams<ForceDensityMaterial>()
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

ForceDensityMaterial::ForceDensityMaterial(const InputParameters & parameters) :
   DerivativeMaterialInterface<Material>(parameters),
   _c(coupledValue("c")),
   _c_name(getVar("c", 0)->name()),
   _ceq(getParam<Real>("ceq")),
   _cgb(getParam<Real>("cgb")),
   _k(getParam<Real>("k")),
   _ncrys(coupledComponents("etas")), //determine number of grains from the number of names passed in.
   _vals(_ncrys), //Size variable arrays
   _grad_vals(_ncrys),
   _product_etas(_ncrys),
   _sum_grad_etas(_ncrys),
   _dF(declareProperty<std::vector<RealGradient> >("force_density")),
   _dFdc(declarePropertyDerivative<std::vector<RealGradient> >("force_density", _c_name))
{
  //Loop through grains and load coupled variables into the arrays
  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _vals[i] = &coupledValue("etas", i);
    _grad_vals[i] = &coupledGradient("etas", i);
  }
}

void
ForceDensityMaterial::computeQpProperties()
{
  _dF[_qp].resize(_ncrys);
  _dFdc[_qp].resize(_ncrys);

  for (unsigned int i = 0; i < _ncrys; ++i)
  {
    _product_etas[i] = 0.0;
    _sum_grad_etas[i] = 0.0;

    for (unsigned int j = 0; j < _ncrys; ++j)
    {
      if (j != i)
      {
        _product_etas[i] = (*_vals[i])[_qp] * (*_vals[j])[_qp] >= _cgb? 1 : 0; //Sum all other order parameters
        _sum_grad_etas[i] += _product_etas[i] * ((*_grad_vals[i])[_qp] - (*_grad_vals[j])[_qp]);
      }
    }
  _dF[_qp][i] = _k * (_c[_qp] - _ceq) * _sum_grad_etas[i];
  _dFdc[_qp][i] = _k * _sum_grad_etas[i];
 }
}

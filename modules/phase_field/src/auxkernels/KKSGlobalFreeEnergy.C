/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "KKSGlobalFreeEnergy.h"

template<>
InputParameters validParams<KKSGlobalFreeEnergy>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<MaterialPropertyName>("fa_name", "Base name of the free energy function F (f_name in the corresponding derivative function material)");
  params.addRequiredParam<MaterialPropertyName>("fb_name", "Base name of the free energy function F (f_name in the corresponding derivative function material)");
  params.addParam<MaterialPropertyName>("h_name", "h", "Base name for the switching function h(eta)");
  params.addParam<MaterialPropertyName>("g_name", "g", "Base name for the double well function g(eta)");
  params.addRequiredParam<Real>("w", "Double well height parameter");
  return params;
}

KKSGlobalFreeEnergy::KKSGlobalFreeEnergy(const InputParameters & parameters) :
    AuxKernel(parameters),
    _prop_fa(getMaterialProperty<Real>("fa_name")),
    _prop_fb(getMaterialProperty<Real>("fb_name")),
    _prop_h(getMaterialProperty<Real>("h_name")),
    _prop_g(getMaterialProperty<Real>("g_name")),
    _w(getParam<Real>("w"))
{
}

Real
KKSGlobalFreeEnergy::computeValue()
{
  Real h = _prop_h[_qp];
  return _prop_fa[_qp] * h + _prop_fb[_qp] * (1.0 - h) + _w * _prop_g[_qp];
}


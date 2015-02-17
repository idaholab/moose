#include "KKSGlobalFreeEnergy.h"

template<>
InputParameters validParams<KKSGlobalFreeEnergy>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addRequiredParam<std::string>("fa_name", "Base name of the free energy function F (f_name in the corresponding derivative function material)");
  params.addRequiredParam<std::string>("fb_name", "Base name of the free energy function F (f_name in the corresponding derivative function material)");
  params.addParam<std::string>("h_name", "h", "Base name for the switching function h(eta)");
  params.addParam<std::string>("g_name", "g", "Base name for the double well function g(eta)");
  params.addRequiredParam<Real>("w", "Double well height parameter");
  return params;
}

KKSGlobalFreeEnergy::KKSGlobalFreeEnergy(const std::string & name, InputParameters parameters) :
    AuxKernel(name, parameters),
    _prop_fa(getMaterialProperty<Real>(getParam<std::string>("fa_name"))),
    _prop_fb(getMaterialProperty<Real>(getParam<std::string>("fb_name"))),
    _prop_h(getMaterialProperty<Real>(getParam<std::string>("h_name"))),
    _prop_g(getMaterialProperty<Real>(getParam<std::string>("g_name"))),
    _w(getParam<Real>("w"))
{
}

Real
KKSGlobalFreeEnergy::computeValue()
{
  Real h = _prop_h[_qp];
  return _prop_fa[_qp] * h + _prop_fb[_qp] * (1.0 - h) + _w * _prop_g[_qp];
}

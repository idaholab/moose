#include "ACFracInterfaceRateMaterial.h"

template<>
InputParameters validParams<ACFracInterfaceRateMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addParam<Real>("L0", 1.0, "The mobility parameter for order parameter");
  params.addParam<Real>("l", 1.0, "Characteristic length parameter");
  return params;
}

ACFracInterfaceRateMaterial::ACFracInterfaceRateMaterial(const std::string & name,
                                                       InputParameters parameters) :
    Material(name, parameters),
    _L(declareProperty<Real>("L")),
    _kappa_op(declareProperty<Real>("kappa_op")),
    _L0(getParam<Real>("L0")),
    _l(getParam<Real>("l"))
{
}

void
ACFracInterfaceRateMaterial::computeQpProperties()
{
  _kappa_op[_qp] = _l;
  _L[_qp] = _L0;
}

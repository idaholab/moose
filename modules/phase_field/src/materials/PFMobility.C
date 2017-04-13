/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PFMobility.h"

// libmesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<PFMobility>()
{
  InputParameters params = validParams<Material>();
  params.addRequiredParam<Real>("mob", "The mobility value");
  params.addParam<Real>("kappa", 1.0, "The kappa parameter for the vacancy concentration");
  return params;
}

PFMobility::PFMobility(const InputParameters & parameters)
  : Material(parameters),
    _M(declareProperty<Real>("M")),
    _grad_M(declareProperty<RealGradient>("grad_M")),
    _kappa_c(declareProperty<Real>("kappa_c")),
    _mob(getParam<Real>("mob")),
    _kappa(getParam<Real>("kappa"))
{
}

void
PFMobility::computeProperties()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    _M[qp] = _mob;
    _grad_M[qp] = 0.0;
    _kappa_c[qp] = _kappa;
  }
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PFMobility.h"

#include "libmesh/quadrature.h"

InputParameters
PFMobility::validParams()
{
  InputParameters params = Material::validParams();
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

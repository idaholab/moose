//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceOrientationMultiphaseMaterial.h"
#include "MooseMesh.h"
#include "MathUtils.h"

registerMooseObject("PhaseFieldApp", InterfaceOrientationMultiphaseMaterial);

template <>
InputParameters
validParams<InterfaceOrientationMultiphaseMaterial>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "This Material accounts for the the orientation dependence"
      "of interfacial energy for multi-phase multi-order parameter phase-field model.");
  params.addRequiredParam<MaterialPropertyName>("kappa_name",
                                                "Name of the kappa for the given phase");
  params.addRequiredParam<MaterialPropertyName>(
      "dkappadgrad_etaa_name", "Name of the derivative of kappa wrt the gradient of eta");
  params.addRequiredParam<MaterialPropertyName>(
      "d2kappadgrad_etaa_name", "Name of the second derivative of kappa wrt the gradient of eta");
  params.addParam<Real>(
      "anisotropy_strength", 0.04, "Strength of the anisotropy (typically < 0.05)");
  params.addParam<unsigned int>("mode_number", 4, "Mode number for anisotropy");
  params.addParam<Real>("reference_angle", 90, "Reference angle for defining anistropy in degrees");
  params.addParam<Real>("kappa_bar", 0.1125, "Average value of the interface parameter kappa");
  params.addRequiredCoupledVar("etaa", "Order parameter for the current phase alpha");
  params.addRequiredCoupledVar("etab", "Order parameter for the neighboring phase beta");
  return params;
}

InterfaceOrientationMultiphaseMaterial::InterfaceOrientationMultiphaseMaterial(
    const InputParameters & parameters)
  : Material(parameters),
    _kappa_name(getParam<MaterialPropertyName>("kappa_name")),
    _dkappadgrad_etaa_name(getParam<MaterialPropertyName>("dkappadgrad_etaa_name")),
    _d2kappadgrad_etaa_name(getParam<MaterialPropertyName>("d2kappadgrad_etaa_name")),
    _delta(getParam<Real>("anisotropy_strength")),
    _j(getParam<unsigned int>("mode_number")),
    _theta0(getParam<Real>("reference_angle")),
    _kappa_bar(getParam<Real>("kappa_bar")),
    _kappa(declareProperty<Real>(_kappa_name)),
    _dkappadgrad_etaa(declareProperty<RealGradient>(_dkappadgrad_etaa_name)),
    _d2kappadgrad_etaa(declareProperty<RealTensorValue>(_d2kappadgrad_etaa_name)),
    _etaa(coupledValue("etaa")),
    _grad_etaa(coupledGradient("etaa")),
    _etab(coupledValue("etab")),
    _grad_etab(coupledGradient("etab"))
{
  // this currently only works in 2D simulations
  if (_mesh.dimension() != 2)
    mooseError("InterfaceOrientationMultiphaseMaterial requires a two-dimensional mesh.");
}

void
InterfaceOrientationMultiphaseMaterial::computeQpProperties()
{
  const Real tol = 1e-9;
  const Real cutoff = 1.0 - tol;

  // cosine of the gradient orientation angle
  Real n = 0.0;
  // normal direction of the interface
  const RealGradient nd = _grad_etaa[_qp] - _grad_etab[_qp];
  const Real nx = nd(0);
  const Real ny = nd(1);
  const Real n2x = nd(0) * nd(0);
  const Real n2y = nd(1) * nd(1);
  const Real nsq = nd.norm_sq();
  const Real n2sq = nsq * nsq;
  if (nsq > tol)
    n = nx / std::sqrt(nsq);

  if (n > cutoff)
    n = cutoff;

  if (n < -cutoff)
    n = -cutoff;

  const Real angle = std::acos(n) * MathUtils::sign(ny);

  // Compute derivative of angle with respect to grad_op
  RealGradient _dangledgrad_etaa;
  if (nsq > tol)
  {
    _dangledgrad_etaa(0) = -ny;
    _dangledgrad_etaa(1) = nx;
    _dangledgrad_etaa *= (MathUtils::sign(ny) / nsq);
  }

  Real anglediff = _j * (angle - _theta0 * libMesh::pi / 180.0);
  // Calculate interfacial parameter kappa and its derivatives wrt angle
  _kappa[_qp] =
      _kappa_bar * (1.0 + _delta * std::cos(anglediff)) * (1.0 + _delta * std::cos(anglediff));

  Real _dkappadangle =
      -2.0 * _kappa_bar * _delta * _j * (1.0 + _delta * std::cos(anglediff)) * std::sin(anglediff);

  Real _d2kappadangle =
      2.0 * _kappa_bar * _delta * _delta * _j * _j * std::sin(anglediff) * std::sin(anglediff) -
      2.0 * _kappa_bar * _delta * _j * _j * (1.0 + _delta * std::cos(anglediff)) *
          std::cos(anglediff);

  RealTensorValue _dangledgrad_etaasq;
  if (nsq > tol)
  {
    _dangledgrad_etaasq(0, 0) = n2y;
    _dangledgrad_etaasq(0, 1) = -(nx * ny);
    _dangledgrad_etaasq(1, 0) = -(nx * ny);
    _dangledgrad_etaasq(1, 1) = n2x;
    _dangledgrad_etaasq /= n2sq;
  }

  RealTensorValue _d2angledgrad_etaa;
  if (nsq > tol)
  {
    _d2angledgrad_etaa(0, 0) = 2.0 * nx * ny;
    _d2angledgrad_etaa(0, 1) = n2y - n2x;
    _d2angledgrad_etaa(1, 0) = n2y - n2x;
    _d2angledgrad_etaa(1, 1) = -2.0 * nx * ny;
    _d2angledgrad_etaa *= (MathUtils::sign(ny) / n2sq);
  }

  // Compute derivatives of kappa and its derivatives wrt grad_eta
  _dkappadgrad_etaa[_qp] = _dkappadangle * _dangledgrad_etaa;
  _d2kappadgrad_etaa[_qp] =
      _d2kappadangle * _dangledgrad_etaasq + _dkappadangle * _d2angledgrad_etaa;
}

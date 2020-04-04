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

InputParameters
InterfaceOrientationMultiphaseMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "This Material accounts for the the orientation dependence "
      "of interfacial energy for multi-phase multi-order parameter phase-field model.");
  params.addRequiredParam<MaterialPropertyName>("kappa_name",
                                                "Name of the kappa for the given phase");
  params.addRequiredParam<MaterialPropertyName>(
      "dkappadgrad_etaa_name", "Name of the derivative of kappa w.r.t. the gradient of eta");
  params.addRequiredParam<MaterialPropertyName>(
      "d2kappadgrad_etaa_name",
      "Name of the second derivative of kappa w.r.t. the gradient of eta");
  params.addParam<Real>(
      "anisotropy_strength", 0.04, "Strength of the anisotropy (typically < 0.05)");
  params.addParam<unsigned int>("mode_number", 4, "Mode number for anisotropy");
  params.addParam<Real>(
      "reference_angle", 90, "Reference angle for defining anisotropy in degrees");
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

  // Normal direction of the interface
  Real n = 0.0;
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

  // Calculate the orientation angle
  const Real angle = std::acos(n) * MathUtils::sign(ny);

  // Compute derivatives of the angle wrt n
  const Real dangledn = -MathUtils::sign(ny) / std::sqrt(1.0 - n * n);
  const Real d2angledn2 = -MathUtils::sign(ny) * n / (1.0 - n * n) / std::sqrt(1.0 - n * n);

  // Compute derivative of n wrt grad_eta
  RealGradient dndgrad_etaa;
  if (nsq > tol)
  {
    dndgrad_etaa(0) = ny * ny;
    dndgrad_etaa(1) = -nx * ny;
    dndgrad_etaa /= nsq * std::sqrt(nsq);
  }

  // Compute the square of dndgrad_etaa
  RealTensorValue dndgrad_etaa_sq;
  if (nsq > tol)
  {
    dndgrad_etaa_sq(0, 0) = n2y * n2y;
    dndgrad_etaa_sq(0, 1) = -nx * ny * n2y;
    dndgrad_etaa_sq(1, 0) = -nx * ny * n2y;
    dndgrad_etaa_sq(1, 1) = n2x * n2y;
    dndgrad_etaa_sq /= n2sq * nsq;
  }

  // Compute the second derivative of n wrt grad_eta
  RealTensorValue d2ndgrad_etaa2;
  if (nsq > tol)
  {
    d2ndgrad_etaa2(0, 0) = -3.0 * nx * n2y;
    d2ndgrad_etaa2(0, 1) = -ny * n2y + 2.0 * ny * n2x;
    d2ndgrad_etaa2(1, 0) = -ny * n2y + 2.0 * ny * n2x;
    d2ndgrad_etaa2(1, 1) = -nx * n2x + 2.0 * nx * n2y;
    d2ndgrad_etaa2 /= n2sq * std::sqrt(nsq);
  }

  // Calculate interfacial coefficient kappa and its derivatives wrt the angle
  Real anglediff = _j * (angle - _theta0 * libMesh::pi / 180.0);
  _kappa[_qp] =
      _kappa_bar * (1.0 + _delta * std::cos(anglediff)) * (1.0 + _delta * std::cos(anglediff));
  Real dkappadangle =
      -2.0 * _kappa_bar * _delta * _j * (1.0 + _delta * std::cos(anglediff)) * std::sin(anglediff);
  Real d2kappadangle =
      2.0 * _kappa_bar * _delta * _delta * _j * _j * std::sin(anglediff) * std::sin(anglediff) -
      2.0 * _kappa_bar * _delta * _j * _j * (1.0 + _delta * std::cos(anglediff)) *
          std::cos(anglediff);

  // Compute derivatives of kappa wrt grad_eta
  _dkappadgrad_etaa[_qp] = dkappadangle * dangledn * dndgrad_etaa;
  _d2kappadgrad_etaa[_qp] = d2kappadangle * dangledn * dangledn * dndgrad_etaa_sq +
                            dkappadangle * d2angledn2 * dndgrad_etaa_sq +
                            dkappadangle * dangledn * d2ndgrad_etaa2;
}

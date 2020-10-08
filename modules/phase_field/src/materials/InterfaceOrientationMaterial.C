//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceOrientationMaterial.h"
#include "MooseMesh.h"
#include "MathUtils.h"

registerMooseObject("PhaseFieldApp", InterfaceOrientationMaterial);

InputParameters
InterfaceOrientationMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("2D interfacial anisotropy");
  params.addParam<Real>(
      "anisotropy_strength", 0.04, "Strength of the anisotropy (typically < 0.05)");
  params.addParam<unsigned int>("mode_number", 6, "Mode number for anisotropy");
  params.addParam<Real>(
      "reference_angle", 90, "Reference angle for defining anisotropy in degrees");
  params.addParam<Real>("eps_bar", 0.01, "Average value of the interface parameter epsilon");
  params.addRequiredCoupledVar("op", "Order parameter defining the solid phase");
  return params;
}

InterfaceOrientationMaterial::InterfaceOrientationMaterial(const InputParameters & parameters)
  : Material(parameters),
    _delta(getParam<Real>("anisotropy_strength")),
    _j(getParam<unsigned int>("mode_number")),
    _theta0(getParam<Real>("reference_angle")),
    _eps_bar(getParam<Real>("eps_bar")),
    _eps(declareProperty<Real>("eps")),
    _deps(declareProperty<Real>("deps")),
    _depsdgrad_op(declareProperty<RealGradient>("depsdgrad_op")),
    _ddepsdgrad_op(declareProperty<RealGradient>("ddepsdgrad_op")),
    _op(coupledValue("op")),
    _grad_op(coupledGradient("op"))
{
  // this currently only works in 2D simulations
  if (_mesh.dimension() != 2)
    mooseError("InterfaceOrientationMaterial requires a two-dimensional mesh.");
}

void
InterfaceOrientationMaterial::computeQpProperties()
{
  const Real tol = 1e-9;
  const Real cutoff = 1.0 - tol;

  // cosine of the gradient orientation angle
  Real n = 0.0;
  const Real nsq = _grad_op[_qp].norm_sq();
  if (nsq > tol)
    n = _grad_op[_qp](0) / std::sqrt(nsq);

  if (n > cutoff)
    n = cutoff;

  if (n < -cutoff)
    n = -cutoff;

  const Real angle = std::acos(n) * MathUtils::sign(_grad_op[_qp](1));

  // Compute derivative of angle wrt n
  const Real dangledn = -MathUtils::sign(_grad_op[_qp](1)) / std::sqrt(1.0 - n * n);

  // Compute derivative of n with respect to grad_op
  RealGradient dndgrad_op;
  if (nsq > tol)
  {
    dndgrad_op(0) = _grad_op[_qp](1) * _grad_op[_qp](1);
    dndgrad_op(1) = -_grad_op[_qp](0) * _grad_op[_qp](1);
    dndgrad_op /= (_grad_op[_qp].norm_sq() * _grad_op[_qp].norm());
  }

  // Calculate interfacial parameter epsilon and its derivatives
  _eps[_qp] = _eps_bar * (_delta * std::cos(_j * (angle - _theta0 * libMesh::pi / 180.0)) + 1.0);
  _deps[_qp] = -_eps_bar * _delta * _j * std::sin(_j * (angle - _theta0 * libMesh::pi / 180.0));
  Real d2eps =
      -_eps_bar * _delta * _j * _j * std::cos(_j * (angle - _theta0 * libMesh::pi / 180.0));

  // Compute derivatives of epsilon and its derivative wrt grad_op
  _depsdgrad_op[_qp] = _deps[_qp] * dangledn * dndgrad_op;
  _ddepsdgrad_op[_qp] = d2eps * dangledn * dndgrad_op;
}

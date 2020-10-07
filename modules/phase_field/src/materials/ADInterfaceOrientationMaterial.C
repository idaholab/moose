//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADInterfaceOrientationMaterial.h"
#include "MooseMesh.h"
#include "MathUtils.h"

registerMooseObject("PhaseFieldApp", ADInterfaceOrientationMaterial);

InputParameters
ADInterfaceOrientationMaterial::validParams()
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

ADInterfaceOrientationMaterial::ADInterfaceOrientationMaterial(const InputParameters & parameters)
  : Material(parameters),
    _delta(getParam<Real>("anisotropy_strength")),
    _j(getParam<unsigned int>("mode_number")),
    _theta0(getParam<Real>("reference_angle")),
    _eps_bar(getParam<Real>("eps_bar")),
    _eps(declareADProperty<Real>("eps")),
    _deps(declareADProperty<Real>("deps")),
    _op(adCoupledValue("op")),
    _grad_op(adCoupledGradient("op"))
{
  // this currently only works in 2D simulations
  if (_mesh.dimension() != 2)
    mooseError("ADInterfaceOrientationMaterial requires a two-dimensional mesh.");
}

void
ADInterfaceOrientationMaterial::computeQpProperties()
{
  const Real tol = 1e-9;
  const Real cutoff = 1.0 - tol;

  // cosine of the gradient orientation angle
  ADReal n = 0.0;
  const ADReal nsq = _grad_op[_qp].norm_sq();
  if (nsq > tol)
    n = std::max(-cutoff, std::min(_grad_op[_qp](0) / std::sqrt(nsq), cutoff));

  // Calculate interfacial parameter epsilon and its derivative
  const ADReal angle = std::acos(n) * MathUtils::sign(_grad_op[_qp](1));
  _eps[_qp] = _eps_bar * (_delta * std::cos(_j * (angle - _theta0 * libMesh::pi / 180.0)) + 1.0);
  _deps[_qp] = -_eps_bar * _delta * _j * std::sin(_j * (angle - _theta0 * libMesh::pi / 180.0));
}

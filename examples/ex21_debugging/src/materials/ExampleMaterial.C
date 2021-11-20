//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleMaterial.h"

registerMooseObject("ExampleApp", ExampleMaterial);

InputParameters
ExampleMaterial::validParams()
{
  InputParameters params = Material::validParams();

  // Vectors for Linear Interpolation
  params.addRequiredParam<std::vector<Real>>(
      "independent_vals", "The vector of indepedent values for building the piecewise function");
  params.addRequiredParam<std::vector<Real>>(
      "dependent_vals", "The vector of depedent values for building the piecewise function");

  params.addCoupledVar(
      "diffusion_gradient",
      "The gradient of this variable will be used to compute a velocity vector property.");

  return params;
}

ExampleMaterial::ExampleMaterial(const InputParameters & parameters)
  : Material(parameters),
    // Declare that this material is going to provide a Real
    // valued property named "diffusivity" that Kernels can use.
    _diffusivity(declareProperty<Real>("diffusivity")),

    // Declare that this material is going to provide a RealGradient
    // valued property named "convection_velocity" that Kernels can use.
    _convection_velocity(declareProperty<RealGradient>("convection_velocity")),

    // Get the reference to the variable coupled into this Material
    _diffusion_gradient(isCoupled("diffusion_gradient") ? coupledGradient("diffusion_gradient")
                                                        : _grad_zero),

    _piecewise_func(getParam<std::vector<Real>>("independent_vals"),
                    getParam<std::vector<Real>>("dependent_vals"))
{
}

void
ExampleMaterial::computeQpProperties()
{
  // We will compute the diffusivity based on the Linear Interpolation of the provided vectors in
  // the z-direction
  _diffusivity[_qp] = _piecewise_func.sample(_coord[2]);

  _convection_velocity[_qp] = _diffusion_gradient[_qp];
}

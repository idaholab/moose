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

  // Allow users to specify vectors defining the points of a piecewise function formed via linear
  // interpolation.
  params.addRequiredParam<std::vector<Real>>(
      "independent_vals",
      "The vector of z-coordinate values for a piecewise function's independent variable");
  params.addRequiredParam<std::vector<Real>>(
      "dependent_vals", "The vector of diffusivity values for a piecewise function's dependent");

  // Allow the user to specify which independent variable's gradient to use for calculating the
  // convection velocity property:
  params.addCoupledVar(
      "diffusion_gradient",
      "The gradient of this variable will be used to compute a velocity vector property.");

  return params;
}

ExampleMaterial::ExampleMaterial(const InputParameters & parameters)
  : Material(parameters),
    // Declare that this material is going to provide a Real value typed
    // material property named "diffusivity" that Kernels and other objects can use.
    // This property is "bound" to the class's "_diffusivity" member.
    _diffusivity(declareProperty<Real>("diffusivity")),

    // Also declare a second "convection_velocity" RealGradient value typed property.
    _convection_velocity(declareProperty<RealGradient>("convection_velocity")),

    // Get the reference to the variable coupled into this Material.
    _diffusion_gradient(isCoupled("diffusion_gradient") ? coupledGradient("diffusion_gradient")
                                                        : _grad_zero),

    // Initialize our piecewise function helper with the user-specified interpolation points.
    _piecewise_func(getParam<std::vector<Real>>("independent_vals"),
                    getParam<std::vector<Real>>("dependent_vals"))
{
}

void
ExampleMaterial::computeQpProperties()
{
  // Diffusivity is the value of the interpolated piece-wise function described by the user
  _diffusivity[_qp] = _piecewise_func.sample(_q_point[_qp](2));

  // Convection velocity is set equal to the gradient of the variable set by the user.
  _convection_velocity[_qp] = _diffusion_gradient[_qp];
}

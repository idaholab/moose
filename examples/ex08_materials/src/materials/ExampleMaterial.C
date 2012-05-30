/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ExampleMaterial.h"

template<>
InputParameters validParams<ExampleMaterial>()
{
  InputParameters params = validParams<Material>();

  // Vectors for Linear Interpolation
  params.addRequiredParam<std::vector<Real> >("independent_vals", "The vector of indepedent values for building the piecewise function");
  params.addRequiredParam<std::vector<Real> >("dependent_vals", "The vector of depedent values for building the piecewise function");

  params.addCoupledVar("diffusion_gradient", "The gradient of this variable will be used to compute a velocity vector property.");

  return params;
}

ExampleMaterial::ExampleMaterial(const std::string & name,
                                 InputParameters parameters) :
    Material(name, parameters),
    // Declare that this material is going to provide a Real
    // valued property named "diffusivity" that Kernels can use.
    _diffusivity(declareProperty<Real>("diffusivity")),

    // Declare that this material is going to provide a RealGradient
    // valued property named "convection_velocity" that Kernels can use.
    _convection_velocity(declareProperty<RealGradient>("convection_velocity")),

    // Get the reference to the variable coupled into this Material
    _diffusion_gradient(isCoupled("diffusion_gradient") ? coupledGradient("diffusion_gradient") : _grad_zero),

    _piecewise_func(getParam<std::vector<Real> >("independent_vals"),
                    getParam<std::vector<Real> >("dependent_vals"))
{}

void
ExampleMaterial::computeQpProperties()
{
  // We will compute the diffusivity based on the Linear Interpolation of the provided vectors in the z-direction
  _diffusivity[_qp] = _piecewise_func.sample(_coord[2]);

  _convection_velocity[_qp] = _diffusion_gradient[_qp];
}

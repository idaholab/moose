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

#include "ExampleDiffusion.h"

// If we use a material pointer we need to include the
// material class
#include "Material.h"

/**
 * This function defines the valid parameters for
 * this Kernel and their default values
 */
template<>
InputParameters validParams<ExampleDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  return params;
}


ExampleDiffusion::ExampleDiffusion(const std::string & name,
                                   InputParameters parameters)
  :Diffusion(name,parameters),
   _diffusivity(getMaterialProperty<Real>("diffusivity"))
{}

Real
ExampleDiffusion::computeQpResidual()
{
  // We're dereferencing the _diffusivity pointer to get to the
  // material properties vector... which gives us one property
  // value per quadrature point.

  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  return _diffusivity[_qp]*Diffusion::computeQpResidual();
}

Real
ExampleDiffusion::computeQpJacobian()
{
  // We're dereferencing the _diffusivity pointer to get to the
  // material properties vector... which gives us one property
  // value per quadrature point.

  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  return _diffusivity[_qp]*Diffusion::computeQpJacobian();
}

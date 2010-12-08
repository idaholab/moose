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

#include "Material.h"

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
  return _diffusivity[_qp]*Diffusion::computeQpResidual();
}

Real
ExampleDiffusion::computeQpJacobian()
{
  return _diffusivity[_qp]*Diffusion::computeQpJacobian();
}

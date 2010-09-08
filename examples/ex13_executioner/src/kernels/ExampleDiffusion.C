#include "ExampleDiffusion.h"

#include "Material.h"

template<>
InputParameters validParams<ExampleDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  return params;
}

ExampleDiffusion::ExampleDiffusion(const std::string & name,
                                   MooseSystem &sys,
                                   InputParameters parameters)
  :Diffusion(name,sys,parameters),
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

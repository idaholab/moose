#include "ExampleDiffusion.h"
#include "Material.h"

template<>
InputParameters validParams<ExampleDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  // Here we will look for a parameter from the input file
  params.addParam<Real>("diffusivity", 1.0, "Diffusivity Coefficient");
  return params;
}

ExampleDiffusion::ExampleDiffusion(const std::string & name,
                                   MooseSystem &sys,
                                   InputParameters parameters)
  :Diffusion(name,sys,parameters),
   // Initialize our member variable based on a default or input file
   _diffusivity(getParam<Real>("diffusivity"))
{}

Real
ExampleDiffusion::computeQpResidual()
{
  return _diffusivity*Diffusion::computeQpResidual();
}

Real
ExampleDiffusion::computeQpJacobian()
{
  return _diffusivity*Diffusion::computeQpJacobian();
}

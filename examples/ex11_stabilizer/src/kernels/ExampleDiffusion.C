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


ExampleDiffusion::ExampleDiffusion(std::string name,
                                   MooseSystem &sys,
                                   InputParameters parameters)
  :Diffusion(name,sys,parameters),
   _diffusivity(getRealMaterialProperty("diffusivity"))
{}

Real
ExampleDiffusion::computeQpResidual()
{
  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  return _diffusivity[_qp]*Diffusion::computeQpResidual();
}

Real
ExampleDiffusion::computeQpJacobian()
{
  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  return _diffusivity[_qp]*Diffusion::computeQpJacobian();
}

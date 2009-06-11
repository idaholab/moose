#include "ExampleDiffusion.h"

// If we use a material pointer we need to include the
// material class
#include "Material.h"

void
ExampleDiffusion::subdomainSetup()
{
  // We are grabbing the "diffusivity" material property
  // that will be used on this subdomain.
  // _material automatically points to the current material
  _diffusivity = &_material->getRealProperty("diffusivity");
}

Real
ExampleDiffusion::computeQpResidual()
{
  // We're dereferencing the _diffusivity pointer to get to the
  // material properties vector... which gives us one property
  // value per quadrature point.

  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  return (*_diffusivity)[_qp]*Diffusion::computeQpResidual();
}

Real
ExampleDiffusion::computeQpJacobian()
{
  // We're dereferencing the _diffusivity pointer to get to the
  // material properties vector... which gives us one property
  // value per quadrature point.

  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  return (*_diffusivity)[_qp]*Diffusion::computeQpJacobian();
}

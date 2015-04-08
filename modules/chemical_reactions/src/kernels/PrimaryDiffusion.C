/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PrimaryDiffusion.h"

// If we use a material pointer we need to include the
// material class
#include "Material.h"

template<>
InputParameters validParams<PrimaryDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  return params;
}

PrimaryDiffusion::PrimaryDiffusion(const std::string & name, InputParameters parameters)
  :Diffusion(name,parameters),
   // We are grabbing the "diffusivity" material property
   _diffusivity(getMaterialProperty<Real>("diffusivity"))
{
}

Real
PrimaryDiffusion::computeQpResidual()
{
  // We're dereferencing the _diffusivity pointer to get to the
  // material properties vector... which gives us one property
  // value per quadrature point.

  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  //  if (_u[_qp]>=0.0)
    return _diffusivity[_qp]*Diffusion::computeQpResidual();

}

Real
PrimaryDiffusion::computeQpJacobian()
{
  // We're dereferencing the _diffusivity pointer to get to the
  // material properties vector... which gives us one property
  // value per quadrature point.

  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  return _diffusivity[_qp]*Diffusion::computeQpJacobian();
}

Real PrimaryDiffusion::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.0;
}

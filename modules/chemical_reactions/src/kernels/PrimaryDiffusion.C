/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PrimaryDiffusion.h"

template<>
InputParameters validParams<PrimaryDiffusion>()
{
  InputParameters params = validParams<Diffusion>();
  return params;
}

PrimaryDiffusion::PrimaryDiffusion(const InputParameters & parameters) :
    Diffusion(parameters),
    _diffusivity(getMaterialProperty<Real>("diffusivity"))
{
}

Real
PrimaryDiffusion::computeQpResidual()
{
  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  //  if (_u[_qp]>=0.0)
  return _diffusivity[_qp] * Diffusion::computeQpResidual();
}

Real
PrimaryDiffusion::computeQpJacobian()
{
  // Also... we're reusing the Diffusion Kernel's residual
  // so that we don't have to recode that.
  return _diffusivity[_qp] * Diffusion::computeQpJacobian();
}

Real PrimaryDiffusion::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.0;
}

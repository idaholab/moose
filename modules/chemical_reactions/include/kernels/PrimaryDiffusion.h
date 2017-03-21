/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "Diffusion.h"

#ifndef PRIMARYDIFFUSION_H
#define PRIMARYDIFFUSION_H

// Forward Declarations
class PrimaryDiffusion;

template <>
InputParameters validParams<PrimaryDiffusion>();

/**
 * Define the Kernel for a CoupledConvectionReactionSub operator that looks like:
 * grad (diff * grad_u)
 */
class PrimaryDiffusion : public Diffusion
{
public:
  PrimaryDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// Material property of dispersion-diffusion coefficient.
  const MaterialProperty<Real> & _diffusivity;
};

#endif // PRIMARYDIFFUSION_H

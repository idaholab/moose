/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATDIFFUSION_H
#define MATDIFFUSION_H

#include "MatDiffusionBase.h"

/**
 * Isotropic diffusion kernel that takes a diffusion coefficient of type
 * Real. All logic is implemnted in the MatDiffusionBase class
 * template.
 */
class MatDiffusion : public MatDiffusionBase<Real>
{
public:
  MatDiffusion(const InputParameters & parameters);
};

template <>
InputParameters validParams<MatDiffusion>();

#endif // MATDIFFUSION_H

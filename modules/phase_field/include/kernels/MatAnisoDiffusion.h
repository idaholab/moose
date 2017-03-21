/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef MATANISODIFFUSION_H
#define MATANISODIFFUSION_H

#include "MatDiffusionBase.h"

/**
 * Anisotropic diffusion kernel that takes a diffusion coefficient of type
 * RealTensorValue. All logic is implemnted in the MatDiffusionBase class
 * template.
 */
class MatAnisoDiffusion : public MatDiffusionBase<RealTensorValue>
{
public:
  MatAnisoDiffusion(const InputParameters & parameters);
};

template <>
InputParameters validParams<MatAnisoDiffusion>();

#endif // MATANISODIFFUSION_H

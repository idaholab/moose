/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LEVELSETBIMATERIALDIFFUSION_H
#define LEVELSETBIMATERIALDIFFUSION_H

#include "LevelSetBiMaterialBase.h"

// Forward Declarations
class LevelSetBiMaterialDiffusion;
class XFEM;

registerMooseObject("XFEMApp", LevelSetBiMaterialDiffusion);

template <>
InputParameters validParams<LevelSetBiMaterialDiffusion>();

/**
 * Compute the diffusion for bi-materials problem (consisting of two different materials) defined by a 
 * level set function
 *
 */
class LevelSetBiMaterialDiffusion : public LevelSetBiMaterialBase
{
public:
  LevelSetBiMaterialDiffusion(const InputParameters & parameters);

protected:
  virtual void assignQpPropertiesForLevelSetPositive();
  virtual void assignQpPropertiesForLevelSetNegative();

  /// diffusion coefficient for the two separate materials in the bi-material system
  std::vector<const MaterialProperty<Real> *> _bimaterial_diffusion_coefficient;

  /// global diffusion coefficient (switch bi-material diffusion coefficient based on level set values)
  MaterialProperty<Real> & _diffusion_coefficient;
};

#endif // LEVELSETBIMATERIALDIFFUSION_H

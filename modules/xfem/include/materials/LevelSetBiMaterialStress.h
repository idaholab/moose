/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LEVELSETBIMATERIALSTRESS_H
#define LEVELSETBIMATERIALSTRESS_H

#include "LevelSetBiMaterialBase.h"

// Forward Declarations
class LevelSetBiMaterialStress;
class RankTwoTensor;
class RankFourTensor;
class XFEM;

registerMooseObject("XFEMApp", LevelSetBiMaterialStress);

template <>
InputParameters validParams<LevelSetBiMaterialStress>();

/**
 * Compute the stress for bi-materials problem (consisting of two different materials) defined by a
 * level set function
 *
 */
class LevelSetBiMaterialStress : public LevelSetBiMaterialBase
{
public:
  LevelSetBiMaterialStress(const InputParameters & parameters);

protected:
  virtual void assignQpPropertiesForLevelSetPositive();
  virtual void assignQpPropertiesForLevelSetNegative();

  /// vector of stresses for the two separate materials in the bi-material system
  std::vector<const MaterialProperty<RankTwoTensor> *> _bimaterial_stress;

  /// vector of stress derivative respect to strain for the two separate materials in the bi-material system
  std::vector<const MaterialProperty<RankFourTensor> *> _dbimaterial_stress_dstrain;

  /// global stress (switch bi-material stresses based on level set values)
  MaterialProperty<RankTwoTensor> & _stress;

  MaterialProperty<RankFourTensor> & _dstress_dstrain;
};

#endif // LEVELSETBIMATERIALSTRESS_H

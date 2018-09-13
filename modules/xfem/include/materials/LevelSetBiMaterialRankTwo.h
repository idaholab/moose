/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LEVELSETBIMATERIALRANKTWO_H
#define LEVELSETBIMATERIALRANKTWO_H

#include "LevelSetBiMaterialBase.h"
#include "RankTwoTensor.h"

// Forward Declarations
class LevelSetBiMaterialRankTwo;

template <>
InputParameters validParams<LevelSetBiMaterialRankTwo>();

/**
 * Compute a RankTwoTensor material property for bi-materials problem (consisting of two different
 * materials) defined by a level set function
 *
 */
class LevelSetBiMaterialRankTwo : public LevelSetBiMaterialBase
{
public:
  LevelSetBiMaterialRankTwo(const InputParameters & parameters);

protected:
  virtual void assignQpPropertiesForLevelSetPositive() override;
  virtual void assignQpPropertiesForLevelSetNegative() override;

  /// RankTwoTensor Material properties for the two separate materials in the bi-material system
  std::vector<const MaterialProperty<RankTwoTensor> *> _bimaterial_material_prop;

  /// Global RankTwoTensor material property (switch bi-material diffusion coefficient based on level set values)
  MaterialProperty<RankTwoTensor> & _material_prop;
};

#endif // LEVELSETBIMATERIALRANKTWO_H

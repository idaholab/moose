/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LEVELSETBIMATERIALRANKFOUR_H
#define LEVELSETBIMATERIALRANKFOUR_H

#include "LevelSetBiMaterialBase.h"
#include "RankFourTensor.h"

// Forward Declarations
class LevelSetBiMaterialRankFour;

template <>
InputParameters validParams<LevelSetBiMaterialRankFour>();

/**
 * Compute a RankFourTensor material property for bi-materials problem (consisting of two different
 * materials) defined by a level set function
 *
 */
class LevelSetBiMaterialRankFour : public LevelSetBiMaterialBase
{
public:
  LevelSetBiMaterialRankFour(const InputParameters & parameters);

protected:
  virtual void assignQpPropertiesForLevelSetPositive() override;
  virtual void assignQpPropertiesForLevelSetNegative() override;

  /// RankFourTensor Material properties for the two separate materials in the bi-material system
  std::vector<const MaterialProperty<RankFourTensor> *> _bimaterial_material_prop;

  /// Global RankFourTensor material property (switch bi-material diffusion coefficient based on level set values)
  MaterialProperty<RankFourTensor> & _material_prop;
};

#endif // LEVELSETBIMATERIALRANKFOUR_H

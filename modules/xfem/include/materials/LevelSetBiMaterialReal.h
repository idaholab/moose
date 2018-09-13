/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef LEVELSETBIMATERIALREAL_H
#define LEVELSETBIMATERIALREAL_H

#include "LevelSetBiMaterialBase.h"

// Forward Declarations
class LevelSetBiMaterialReal;

template <>
InputParameters validParams<LevelSetBiMaterialReal>();

/**
 * Compute a Real material property for bi-materials problem (consisting of two different materials)
 * defined by a level set function
 *
 */
class LevelSetBiMaterialReal : public LevelSetBiMaterialBase
{
public:
  LevelSetBiMaterialReal(const InputParameters & parameters);

protected:
  virtual void assignQpPropertiesForLevelSetPositive();
  virtual void assignQpPropertiesForLevelSetNegative();

  /// Real Material properties for the two separate materials in the bi-material system
  std::vector<const MaterialProperty<Real> *> _bimaterial_material_prop;

  /// Global Real material property (switch bi-material diffusion coefficient based on level set values)
  MaterialProperty<Real> & _material_prop;
};

#endif // LEVELSETBIMATERIALREAL_H

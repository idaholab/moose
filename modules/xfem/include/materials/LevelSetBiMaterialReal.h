//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

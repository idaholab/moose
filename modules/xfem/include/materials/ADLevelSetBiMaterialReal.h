//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADLevelSetBiMaterialBase.h"

/**
 * Compute a Real material property for bi-materials problem (consisting of two different materials)
 * defined by a level set function
 *
 */
class ADLevelSetBiMaterialReal : public ADLevelSetBiMaterialBase
{
public:
  static InputParameters validParams();

  ADLevelSetBiMaterialReal(const InputParameters & parameters);

protected:
  virtual void assignQpPropertiesForLevelSetPositive() override;
  virtual void assignQpPropertiesForLevelSetNegative() override;

  /// Real Material properties for the two separate materials in the bi-material system
  std::vector<const ADMaterialProperty<Real> *> _bimaterial_material_prop;

  /// Global Real material property (switch bi-material diffusion coefficient based on level set values)
  ADMaterialProperty<Real> & _material_prop;
};

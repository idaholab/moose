/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LevelSetBiMaterialDiffusion.h"

template <>
InputParameters
validParams<LevelSetBiMaterialDiffusion>()
{
  InputParameters params = validParams<LevelSetBiMaterialBase>();
  params.addClassDescription(
      "Compute the diffusion coefficient for two materials defined by a level set function.");
  return params;
}

LevelSetBiMaterialDiffusion::LevelSetBiMaterialDiffusion(const InputParameters & parameters)
  : LevelSetBiMaterialBase(parameters),
    _bimaterial_diffusion_coefficient(2),
    _diffusion_coefficient(declareProperty<Real>(_base_name + "diffusion_coefficient"))
{
  _bimaterial_diffusion_coefficient[0] = &getMaterialProperty<Real>(
      getParam<std::string>("levelset_positive_base") + "_diffusion_coefficient");
  _bimaterial_diffusion_coefficient[1] = &getMaterialProperty<Real>(
      getParam<std::string>("levelset_negative_base") + "_diffusion_coefficient");
}

void
LevelSetBiMaterialDiffusion::assignQpPropertiesForLevelSetPositive()
{
  _diffusion_coefficient[_qp] = (*_bimaterial_diffusion_coefficient[0])[_qp];
}

void
LevelSetBiMaterialDiffusion::assignQpPropertiesForLevelSetNegative()
{
  _diffusion_coefficient[_qp] = (*_bimaterial_diffusion_coefficient[1])[_qp];
}

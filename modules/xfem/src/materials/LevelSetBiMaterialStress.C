/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LevelSetBiMaterialStress.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

template <>
InputParameters
validParams<LevelSetBiMaterialStress>()
{
  InputParameters params = validParams<LevelSetBiMaterialBase>();
  params.addClassDescription(
      "Compute the stress for two materials defined by a level set function.");
  return params;
}

LevelSetBiMaterialStress::LevelSetBiMaterialStress(const InputParameters & parameters)
  : LevelSetBiMaterialBase(parameters),
    _bimaterial_stress(2),
    _dbimaterial_stress_dstrain(2),
    _stress(declareProperty<RankTwoTensor>(_base_name + "stress")),
    _dstress_dstrain(declareProperty<RankFourTensor>(_base_name + "Jacobian_mult"))
{
  _bimaterial_stress[0] = &getMaterialProperty<RankTwoTensor>(
      getParam<std::string>("levelset_positive_base") + "_stress");
  _dbimaterial_stress_dstrain[0] = &getMaterialProperty<RankFourTensor>(
      getParam<std::string>("levelset_positive_base") + "_Jacobian_mult");

  _bimaterial_stress[1] = &getMaterialProperty<RankTwoTensor>(
      getParam<std::string>("levelset_negative_base") + "_stress");
  _dbimaterial_stress_dstrain[1] = &getMaterialProperty<RankFourTensor>(
      getParam<std::string>("levelset_negative_base") + "_Jacobian_mult");
}

void
LevelSetBiMaterialStress::assignQpPropertiesForLevelSetPositive()
{
  _stress[_qp] = (*_bimaterial_stress[0])[_qp];
  _dstress_dstrain[_qp] = (*_dbimaterial_stress_dstrain[0])[_qp];
}

void
LevelSetBiMaterialStress::assignQpPropertiesForLevelSetNegative()
{
  _stress[_qp] = (*_bimaterial_stress[1])[_qp];
  _dstress_dstrain[_qp] = (*_dbimaterial_stress_dstrain[1])[_qp];
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeStrainIncrementBasedStress.h"

registerADMooseObject("TensorMechanicsApp", ADComputeStrainIncrementBasedStress);

defineADLegacyParams(ADComputeStrainIncrementBasedStress);

template <ComputeStage compute_stage>
InputParameters
ADComputeStrainIncrementBasedStress<compute_stage>::validParams()
{
  InputParameters params = ADComputeStressBase<compute_stage>::validParams();
  params.addClassDescription("Compute stress after subtracting inelastic strain increments");
  params.addParam<std::vector<MaterialPropertyName>>("inelastic_strain_names",
                                                     "Names of inelastic strain properties");

  return params;
}

template <ComputeStage compute_stage>
ADComputeStrainIncrementBasedStress<compute_stage>::ADComputeStrainIncrementBasedStress(
    const InputParameters & parameters)
  : ADComputeStressBase<compute_stage>(parameters),
    _elasticity_tensor_name(_base_name + "elasticity_tensor"),
    _elasticity_tensor(getMaterialPropertyByName<RankFourTensor>(_elasticity_tensor_name)),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _mechanical_strain_old(
        getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "mechanical_strain")),
    _inelastic_strain_names(getParam<std::vector<MaterialPropertyName>>("inelastic_strain_names"))
{
  _num_inelastic_strain_models = _inelastic_strain_names.size();

  if (_num_inelastic_strain_models > 0)
  {
    _inelastic_strains.resize(_num_inelastic_strain_models);
    _inelastic_strains_old.resize(_num_inelastic_strain_models);

    for (unsigned int i = 0; i < _num_inelastic_strain_models; ++i)
    {
      _inelastic_strains[i] = &getADMaterialProperty<RankTwoTensor>(_inelastic_strain_names[i]);
      _inelastic_strains_old[i] =
          &getMaterialPropertyOld<RankTwoTensor>(_inelastic_strain_names[i]);
    }
  }
}

template <ComputeStage compute_stage>
void
ADComputeStrainIncrementBasedStress<compute_stage>::computeQpStress()
{
  ADRankTwoTensor elastic_strain_increment =
      (_mechanical_strain[_qp] - _mechanical_strain_old[_qp]);

  for (unsigned int i = 0; i < _num_inelastic_strain_models; ++i)
    elastic_strain_increment -= (*_inelastic_strains[i])[_qp] - (*_inelastic_strains_old[i])[_qp];

  _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * elastic_strain_increment;
}

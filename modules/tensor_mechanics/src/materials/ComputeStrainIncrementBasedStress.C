//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeStrainIncrementBasedStress.h"

template <>
InputParameters
validParams<ComputeStrainIncrementBasedStress>()
{
  InputParameters params = validParams<ComputeStressBase>();
  params.addClassDescription("Compute stress after subtracting inelastic strain increments");
  params.addParam<std::vector<MaterialPropertyName>>("inelastic_strain_names",
                                                     "Names of inelastic strain properties");

  return params;
}

ComputeStrainIncrementBasedStress::ComputeStrainIncrementBasedStress(
    const InputParameters & parameters)
  : ComputeStressBase(parameters),
    _stress_old(getMaterialPropertyOld<RankTwoTensor>(_base_name + "stress")),
    _mechanical_strain(getMaterialPropertyByName<RankTwoTensor>(_base_name + "mechanical_strain")),
    _mechanical_strain_old(
        getMaterialPropertyOldByName<RankTwoTensor>(_base_name + "mechanical_strain")),
    _property_names(getParam<std::vector<MaterialPropertyName>>("inelastic_strain_names"))
{
  _num_property = _property_names.size();

  if (_num_property > 0)
  {
    _inelastic_strains.resize(_num_property);
    _inelastic_strains_old.resize(_num_property);

    for (unsigned int i = 0; i < _num_property; ++i)
    {
      _inelastic_strains[i] = &getMaterialProperty<RankTwoTensor>(_property_names[i]);
      _inelastic_strains_old[i] = &getMaterialPropertyOld<RankTwoTensor>(_property_names[i]);
    }
  }
}

void
ComputeStrainIncrementBasedStress::computeQpStress()
{
  RankTwoTensor elastic_strain_increment = (_mechanical_strain[_qp] - _mechanical_strain_old[_qp]);

  for (unsigned int i = 0; i < _num_property; ++i)
    elastic_strain_increment -= (*_inelastic_strains[i])[_qp] - (*_inelastic_strains_old[i])[_qp];

  _stress[_qp] = _stress_old[_qp] + _elasticity_tensor[_qp] * elastic_strain_increment;

  computeQpJacobian();
}

void
ComputeStrainIncrementBasedStress::computeQpJacobian()
{
  _Jacobian_mult[_qp] = _elasticity_tensor[_qp];
}

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRadialReturnCreepStressUpdateBase.h"
#include "RankTwoTensor.h"

defineADValidParams(
    ADRadialReturnCreepStressUpdateBase,
    ADRadialReturnStressUpdate,
    params.set<std::string>("effective_inelastic_strain_name") = "effective_creep_strain";);

template <ComputeStage compute_stage>
ADRadialReturnCreepStressUpdateBase<compute_stage>::ADRadialReturnCreepStressUpdateBase(
    const InputParameters & parameters)
  : ADRadialReturnStressUpdate<compute_stage>(parameters),
    _creep_strain(adDeclareADProperty<RankTwoTensor>(_base_name + "creep_strain")),
    _creep_strain_old(adGetMaterialPropertyOld<RankTwoTensor>(_base_name + "creep_strain"))
{
}

template <ComputeStage compute_stage>
void
ADRadialReturnCreepStressUpdateBase<compute_stage>::initQpStatefulProperties()
{
  _creep_strain[_qp].zero();

  ADRadialReturnStressUpdate<compute_stage>::initQpStatefulProperties();
}

template <ComputeStage compute_stage>
void
ADRadialReturnCreepStressUpdateBase<compute_stage>::propagateQpStatefulProperties()
{
  _creep_strain[_qp] = _creep_strain_old[_qp];

  propagateQpStatefulPropertiesRadialReturn();
}

template <ComputeStage compute_stage>
void
ADRadialReturnCreepStressUpdateBase<compute_stage>::computeStressFinalize(
    const ADRankTwoTensor & plastic_strain_increment)
{
  _creep_strain[_qp] = _creep_strain_old[_qp] + plastic_strain_increment;
}

// explicit instantiation is required for AD base classes
adBaseClass(ADRadialReturnCreepStressUpdateBase);

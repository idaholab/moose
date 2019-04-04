//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeStressBase.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"

defineADValidParams(
    ADComputeStressBase,
    ADMaterial,
    params.addParam<std::string>("base_name",
                                 "Optional parameter that allows the user to define "
                                 "multiple mechanics material systems on the same "
                                 "block, i.e. for multiple phases");
    params.suppressParameter<bool>("use_displaced_mesh"););

template <ComputeStage compute_stage>
ADComputeStressBase<compute_stage>::ADComputeStressBase(const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _base_name(isParamValid("base_name") ? adGetParam<std::string>("base_name") + "_" : ""),
    _mechanical_strain(adGetADMaterialProperty<RankTwoTensor>(_base_name + "mechanical_strain")),
    _deformation_gradient(
        adGetADMaterialProperty<RankTwoTensor>(_base_name + "deformation_gradient")),
    _stress(adDeclareADProperty<RankTwoTensor>(_base_name + "stress")),
    _elastic_strain(adDeclareADProperty<RankTwoTensor>(_base_name + "elastic_strain"))
{

  if (adGetParam<bool>("use_displaced_mesh"))
    mooseError("The stress calculator needs to run on the undisplaced mesh.");
}

template <ComputeStage compute_stage>
void
ADComputeStressBase<compute_stage>::initQpStatefulProperties()
{
  _elastic_strain[_qp].zero();
  _stress[_qp].zero();
}

template <ComputeStage compute_stage>
void
ADComputeStressBase<compute_stage>::computeQpProperties()
{
  computeQpStress();

  // TODO: Add in extra stress (using a list of extra_stress_names analogous to eigenstrain_names)
}

// explicit instantiation is required for AD base classes
adBaseClass(ADComputeStressBase);

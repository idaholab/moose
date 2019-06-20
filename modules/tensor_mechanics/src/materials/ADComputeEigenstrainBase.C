//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADComputeEigenstrainBase.h"

#include "RankTwoTensor.h"

defineADValidParams(
    ADComputeEigenstrainBase,
    ADMaterial,
    params.addParam<std::string>("base_name",
                                 "Optional parameter that allows the user to define "
                                 "multiple mechanics material systems on the same "
                                 "block, i.e. for multiple phases");
    params.addRequiredParam<std::string>(
        "eigenstrain_name",
        "Material property name for the eigenstrain tensor computed "
        "by this model. IMPORTANT: The name of this property must "
        "also be provided to the strain calculator.");
    params.addDeprecatedParam<bool>(
        "incremental_form",
        false,
        "Should the eigenstrain be in incremental form (for incremental models)?",
        "This parameter no longer has any effect. Simply remove it."););

template <ComputeStage compute_stage>
ADComputeEigenstrainBase<compute_stage>::ADComputeEigenstrainBase(
    const InputParameters & parameters)
  : ADMaterial<compute_stage>(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _eigenstrain_name(_base_name + getParam<std::string>("eigenstrain_name")),
    _eigenstrain(declareADProperty<RankTwoTensor>(_eigenstrain_name)),
    _step_zero(declareRestartableData<bool>("step_zero", true))
{
}

template <ComputeStage compute_stage>
void
ADComputeEigenstrainBase<compute_stage>::initQpStatefulProperties()
{
  // This property can be promoted to be stateful by other models that use it,
  // so it needs to be initalized.
  _eigenstrain[_qp].zero();
}

template <ComputeStage compute_stage>
void
ADComputeEigenstrainBase<compute_stage>::computeQpProperties()
{
  if (_t_step >= 1)
    _step_zero = false;

  // Skip the eigenstrain calculation in step zero because no solution is computed during
  // the zeroth step, hence computing the eigenstrain in the zeroth step would result in
  // an incorrect calculation of mechanical_strain, which is stateful.
  if (!_step_zero)
    computeQpEigenstrain();
}

template <ComputeStage compute_stage>
ADReal
ADComputeEigenstrainBase<compute_stage>::computeVolumetricStrainComponent(
    const ADReal volumetric_strain) const
{
  // The engineering strain in a given direction is:
  // epsilon_eng = cbrt(volumetric_strain + 1.0) - 1.0
  //
  // We need to provide this as a logarithmic strain to be consistent with the strain measure
  // used for finite strain:
  // epsilon_log = log(1.0 + epsilon_eng)
  //
  // This can be simplified down to a more direct form:
  // epsilon_log = log(cbrt(volumetric_strain + 1.0))
  // or:
  // epsilon_log = (1/3) log(volumetric_strain + 1.0)

  return std::log(volumetric_strain + 1.0) / 3.0;
}

adBaseClass(ADComputeEigenstrainBase);

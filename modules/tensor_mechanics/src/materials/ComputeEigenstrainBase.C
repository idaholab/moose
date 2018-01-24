//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeEigenstrainBase.h"

#include "RankTwoTensor.h"

template <>
InputParameters
validParams<ComputeEigenstrainBase>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addRequiredParam<std::string>("eigenstrain_name",
                                       "Material property name for the eigenstrain tensor computed "
                                       "by this model. IMPORTANT: The name of this property must "
                                       "also be provided to the strain calculator.");
  params.addDeprecatedParam<bool>(
      "incremental_form",
      false,
      "Should the eigenstrain be in incremental form (for incremental models)?",
      "This parameter no longer has any effect. Simply remove it.");
  return params;
}

ComputeEigenstrainBase::ComputeEigenstrainBase(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _eigenstrain_name(_base_name + getParam<std::string>("eigenstrain_name")),
    _eigenstrain(declareProperty<RankTwoTensor>(_eigenstrain_name)),
    _step_zero(declareRestartableData<bool>("step_zero", true))
{
}

void
ComputeEigenstrainBase::initQpStatefulProperties()
{
  // This property can be promoted to be stateful by other models that use it,
  // so it needs to be initalized.
  _eigenstrain[_qp].zero();
}

void
ComputeEigenstrainBase::computeQpProperties()
{
  if (_t_step >= 1)
    _step_zero = false;

  // Skip the eigenstrain calculation in step zero because no solution is computed during
  // the zeroth step, hence computing the eigenstrain in the zeroth step would result in
  // an incorrect calculation of mechanical_strain, which is stateful.
  if (!_step_zero)
    computeQpEigenstrain();
}

Real
ComputeEigenstrainBase::computeVolumetricStrainComponent(const Real volumetric_strain) const
{

  Real volumetric_strain_comp = std::cbrt(volumetric_strain + 1.0) - 1.0;

  // Convert to logarithmic strain to compute strains to exactly recover
  // volumetric strain in finite strain models
  volumetric_strain_comp = std::log(1.0 + volumetric_strain_comp);

  return volumetric_strain_comp;
}

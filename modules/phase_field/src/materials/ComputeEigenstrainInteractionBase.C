//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeEigenstrainInteractionBase.h"

#include "RankTwoTensor.h"

InputParameters
ComputeEigenstrainInteractionBase::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addRequiredParam<std::string>("eigenstrainInteraction_name",
                                       "Material property name for the eigenstrain interaction tensor computed "
                                       "by this model. IMPORTANT: The name of this property must "
                                       "also be provided to the strain calculator.");
  return params;
}

ComputeEigenstrainInteractionBase::ComputeEigenstrainInteractionBase(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _eigenstrainInteraction_name(_base_name + getParam<std::string>("eigenstrainInteraction_name")),
    _eigenstrainInteraction(declareProperty<RankTwoTensor>(_eigenstrainInteraction_name)),
    _step_zero(declareRestartableData<bool>("step_zero", true))
{
}

void
ComputeEigenstrainInteractionBase::initQpStatefulProperties()
{
  // This property can be promoted to be stateful by other models that use it,
  // so it needs to be initalized.
  _eigenstrainInteraction[_qp].zero();
}

void
ComputeEigenstrainInteractionBase::computeQpProperties()
{
  if (_t_step >= 1)
    _step_zero = false;

  // Skip the eigenstrain interaction calculation in step zero because no solution is computed during
  // the zeroth step, hence computing the eigenstrain in the zeroth step would result in
  // an incorrect calculation of mechanical_strain, which is stateful.
  if (!_step_zero)
    computeQpEigenstrainInteraction();
}

Real
ComputeEigenstrainInteractionBase::computeVolumetricStrainComponent(const Real volumetric_strain) const
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

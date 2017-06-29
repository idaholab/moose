/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeCreepStrainBase.h"

template <>
InputParameters
validParams<ComputeCreepStrainBase>()
{
  InputParameters params = validParams<Material>();
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addParam<bool>("incremental_form",
                        false,
                        "Should the creep strain be in incremental form (for incremental models)?");
  return params;
}

ComputeCreepStrainBase::ComputeCreepStrainBase(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _creep_strain_name(_base_name + "creep_strain"),
    _incremental_form(getParam<bool>("incremental_form")),
    _creep_strain(declareProperty<RankTwoTensor>(_creep_strain_name)),
    _creep_strain_old(_incremental_form ? &declarePropertyOld<RankTwoTensor>(_creep_strain_name)
                                        : NULL),
    _step_zero(declareRestartableData<bool>("step_zero", true))
{
}

void
ComputeCreepStrainBase::initQpStatefulProperties()
{
}

void
ComputeCreepStrainBase::computeQpProperties()
{
  if (_t_step >= 1)
    _step_zero = false;

  if (!_step_zero)
    computeQpCreepStrain();
}

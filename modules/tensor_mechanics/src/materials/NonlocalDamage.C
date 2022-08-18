//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NonlocalDamage.h"

registerMooseObject("TensorMechanicsApp", NonlocalDamage);
registerMooseObject("TensorMechanicsApp", ADNonlocalDamage);

template <bool is_ad>
InputParameters
NonlocalDamageTempl<is_ad>::validParams()
{
  InputParameters params = ScalarDamageBaseTempl<is_ad>::validParams();
  params.addClassDescription(
      "Nonlocal damage model. Given an RadialAverage UO this creates a new damage index that can "
      "be used as for ComputeDamageStress without havign to change existing local damage models.");
  params.addRequiredParam<UserObjectName>("average_UO", "Radial Average user object");
  params.addRequiredParam<MaterialName>("local_damage_model",
                                        "Name of the local damage model used to compute "
                                        "the nonlocal damage index");
  return params;
}

template <bool is_ad>
NonlocalDamageTempl<is_ad>::NonlocalDamageTempl(const InputParameters & parameters)
  : ScalarDamageBaseTempl<is_ad>(parameters),
    GuaranteeConsumer(this),
    _average(this->template getUserObject<RadialAverage>("average_UO").getAverage()),
    _local_damage_model_name(this->template getParam<MaterialName>("local_damage_model")),
    _prev_elem(nullptr)
{
}
template <bool is_ad>
void
NonlocalDamageTempl<is_ad>::initialSetup()
{
  _local_damage_model = dynamic_cast<ScalarDamageBaseTempl<is_ad> *>(
      &this->getMaterialByName(_local_damage_model_name));

  if (!_local_damage_model)
    this->template paramError("damage_model",
                              "Damage Model " + _local_damage_model_name +
                                  " is not compatible with NonlocalDamage model");
}

template <bool is_ad>
void
NonlocalDamageTempl<is_ad>::initQpStatefulProperties()
{
  ScalarDamageBaseTempl<is_ad>::initQpStatefulProperties();
}

template <bool is_ad>
void
NonlocalDamageTempl<is_ad>::updateQpDamageIndex()
{
  // First update underlying local damage model
  _local_damage_model->getQpDamageIndex(_qp);
  // Now update the nonlocal damage model
  // Only update iterator when we change to another element. This is for
  // computational costs related to map lookup.
  if (_prev_elem != _current_elem)
  {
    _average_damage = _average.find(_current_elem->id());
    _prev_elem = _current_elem;
  }
  // Check that we found the new element
  if (_average_damage != _average.end())
    // return max of the old damage or new average damage
    _damage_index[_qp] = std::max(_average_damage->second[_qp], _damage_index_old[_qp]);
  else
    // during startup the map is not made yet or
    // if AMR is used then the new element will not be found but it should
    // already have an old nonlocal damage value that needs to perserved
    _damage_index[_qp] = std::max(0.0, _damage_index_old[_qp]);
}

template class NonlocalDamageTempl<false>;
template class NonlocalDamageTempl<true>;

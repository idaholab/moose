//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "ScalarDamageBase.h"
#include "GuaranteeConsumer.h"
#include "RadialAverage.h"

/**
 * Scalar damage model that defines the damage parameter using a material property
 */
template <bool is_ad>
class NonlocalDamageTempl : public ScalarDamageBaseTempl<is_ad>, public GuaranteeConsumer
{
public:
  static InputParameters validParams();

  NonlocalDamageTempl(const InputParameters & parameters);

  virtual void initialSetup() override;

  virtual void initQpStatefulProperties() override;

protected:
  virtual void updateQpDamageIndex() override;

  // Averaged Material
  // std::string _avg_material_name;
  const RadialAverage::Result & _average;
  RadialAverage::Result::const_iterator _average_damage;

  ///{@ Local damage model needed for updating
  MaterialName _local_damage_model_name;
  ScalarDamageBaseTempl<is_ad> * _local_damage_model;
  ///@}

  // Pointer to last element for comparison for speed
  const Elem * _prev_elem;

  ///@{ Make hierarchy parameters available in this class
  using ScalarDamageBaseTempl<is_ad>::_damage_index;
  using ScalarDamageBaseTempl<is_ad>::_damage_index_name;
  using ScalarDamageBaseTempl<is_ad>::_damage_index_old;
  using ScalarDamageBaseTempl<is_ad>::_damage_index_older;
  using ScalarDamageBaseTempl<is_ad>::_qp;
  using ScalarDamageBaseTempl<is_ad>::_use_old_damage;
  using ScalarDamageBaseTempl<is_ad>::_dt;
  using ScalarDamageBaseTempl<is_ad>::_base_name;
  using ScalarDamageBaseTempl<is_ad>::_maximum_damage_increment;
  using ScalarDamageBaseTempl<is_ad>::_current_elem;
  using ScalarDamageBaseTempl<is_ad>::_JxW;
  using ScalarDamageBaseTempl<is_ad>::_coord;
  ///@}
};

typedef NonlocalDamageTempl<false> NonlocalDamage;
typedef NonlocalDamageTempl<true> ADNonlocalDamage;

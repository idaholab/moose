/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/*                       BlackBear                              */
/*                                                              */
/*           (c) 2017 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#pragma once

#include "MooseTypes.h"
#include "ScalarDamageBase.h"
#include "GuaranteeConsumer.h"
#include "RadialAverage.h"

/**
 * Scalar damage model that defines the damage parameter using a material property
 */
template <bool is_ad>
class NonLocalDamageTempl : public ScalarDamageBaseTempl<is_ad>, public GuaranteeConsumer
{
public:
  static InputParameters validParams();

  NonLocalDamageTempl(const InputParameters & parameters);

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

typedef NonLocalDamageTempl<false> NonLocalDamage;
typedef NonLocalDamageTempl<true> ADNonLocalDamage;

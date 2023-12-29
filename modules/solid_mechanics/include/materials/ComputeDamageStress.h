//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ComputeFiniteStrainElasticStress.h"
#include "ADComputeFiniteStrainElasticStress.h"
#include "DamageBase.h"

// Forward declaration
template <bool is_ad>
using ComputeFiniteStrainElasticStressTempl = typename std::
    conditional<is_ad, ADComputeFiniteStrainElasticStress, ComputeFiniteStrainElasticStress>::type;

/**
 * ComputeDamageStress computes the stress for a damaged elasticity material. This
 * model must be used in conjunction with a damage model (derived from DamageBase)
 */
template <bool is_ad>
class ComputeDamageStressTempl : public ComputeFiniteStrainElasticStressTempl<is_ad>
{
public:
  static InputParameters validParams();

  ComputeDamageStressTempl(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void computeQpStress() override;

  /// Property that stores the time step limit
  MaterialProperty<Real> & _material_timestep_limit;

  /// Pointer to the damage model
  DamageBaseTempl<is_ad> * _damage_model;

  using MaterialBase::_qp;
};

typedef ComputeDamageStressTempl<false> ComputeDamageStress;
typedef ComputeDamageStressTempl<true> ADComputeDamageStress;

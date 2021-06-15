//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "RankFourTensor.h"
#include "RankTwoTensor.h"

// Forward declaration

/**
 * DamageBase is a base class for damage models, which modify the stress tensor
 * computed by another model based on a damage mechanics formulation.
 * These models are designed to be called by another model, so they have
 * compute=false set.
 */
template <bool is_ad>
class DamageBaseTempl : public Material
{
public:
  static InputParameters validParams();

  DamageBaseTempl(const InputParameters & parameters);

  /**
   * Update the internal variable(s) that evolve the damage
   */
  virtual void updateDamage();

  /**
   * Update the current stress tensor for effects of damage.
   * @param stress_new Undamaged stress to be modified by the damage model
   */
  virtual void updateStressForDamage(GenericRankTwoTensor<is_ad> & stress_new) = 0;

  /**
   * Update the material constitutive matrix
   * @param jacobian_mult Material constitutive matrix to be modified for
   * effects of damage
   */
  virtual void updateJacobianMultForDamage(RankFourTensor & jacobian_mult) = 0;

  virtual void computeUndamagedOldStress(RankTwoTensor & stress_old) = 0;
  /**
   * Compute the limiting value of the time step for this material
   * @return Limiting time step
   */
  virtual Real computeTimeStepLimit();

  /**
   * Perform any necessary rotation of internal variables for finite
   * strain.
   * @param rotation_increment The finite-strain rotation increment
   */
  virtual void finiteStrainRotation(const GenericRankTwoTensor<is_ad> & rotation_increment);

  /// Sets the value of the member variable _qp for use in inheriting classes
  void setQp(unsigned int qp);

  ///@{ Retained as empty methods to avoid a warning from Material.C in framework. These methods are unused in all inheriting classes and should not be overwritten.
  void resetQpProperties() final {}
  void resetProperties() final {}
  ///@}

protected:
  /// Base name optionally used as prefix to material tensor names
  const std::string _base_name;
};

typedef DamageBaseTempl<false> DamageBase;
typedef DamageBaseTempl<true> ADDamageBase;

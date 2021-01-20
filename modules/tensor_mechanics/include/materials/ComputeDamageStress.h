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

class DamageBase;

/**
 * ComputeDamageStress computes the stress for a damaged elasticity material. This
 * model must be used in conjunction with a damage model (derived from DamageBase)
 */
class ComputeDamageStress : public ComputeFiniteStrainElasticStress
{
public:
  static InputParameters validParams();

  ComputeDamageStress(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void computeQpStress() override;

  /// Property that stores the time step limit
  MaterialProperty<Real> & _material_timestep_limit;

  /// Pointer to the damage model
  DamageBase * _damage_model;
};

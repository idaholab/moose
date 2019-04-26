//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarDamageBase.h"

// Forward declaration
class ScalarMaterialDamage;

template <>
InputParameters validParams<ScalarMaterialDamage>();

/**
 * Scalar damage model for which the damage is prescribed by another material
 */
class ScalarMaterialDamage : public ScalarDamageBase
{
public:
  ScalarMaterialDamage(const InputParameters & parameters);

protected:
  virtual void updateQpDamageIndex() override;

  ///@{ Material property that provides the damage index
  const MaterialProperty<Real> & _damage_property;
  ///@}
};


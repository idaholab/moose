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
class CombinedScalarDamage;

template <>
InputParameters validParams<CombinedScalarDamage>();

/**
 * Scalar damage model computed as the combination of multiple damage models
 */
class CombinedScalarDamage : public ScalarDamageBase
{
public:
  CombinedScalarDamage(const InputParameters & parameters);

  void initialSetup() override;

protected:
  virtual void updateQpDamageIndex() override;

  enum class CombinationType
  {
    Maximum,
    Product
  };

  /// Type of expansion
  const CombinationType _combination_type;

  std::vector<MaterialName> _damage_models_names;

  std::vector<ScalarDamageBase *> _damage_models;
};


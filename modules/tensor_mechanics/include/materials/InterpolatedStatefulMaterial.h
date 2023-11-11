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
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

/**
 * PowerLawSoftening is a smeared crack softening model that
 * uses a power law equation to soften the tensile response.
 * It is for use with ComputeSmearedCrackingStress.
 */
class InterpolatedStatefulMaterial : public Material
{
public:
  static InputParameters validParams();

  InterpolatedStatefulMaterial(const InputParameters & parameters);

  virtual void computeQpProperties() override;

protected:
  /// Old state
  const std::vector<const VariableValue *> _old_state;

  /// emitted property name
  const MaterialPropertyName _prop_name;

  /// Property type
  enum class PropType
  {
    REAL,
    REALVECTORVALUE,
    RANKTWOTENSOR,
    RANKFOURTENSOR
  } _prop_type;

  MaterialProperty<Real> * _prop_real;
  MaterialProperty<RealVectorValue> * _prop_realvectorvalue;
  MaterialProperty<RankTwoTensor> * _prop_ranktwotensor;
  MaterialProperty<RankFourTensor> * _prop_rankfourtensor;
};

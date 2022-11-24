//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MaterialTensorIntegral.h"

/**
 * This postprocessor computes the volume average of a
 * component of a RankTwoTensor as specified by the user-supplied indices.
 */
template <bool is_ad>
class MaterialTensorAverageTempl : public MaterialTensorIntegralTempl<is_ad>
{
public:
  static InputParameters validParams();

  MaterialTensorAverageTempl(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Domain volume
  Real _volume;
  using MaterialTensorIntegralTempl<is_ad>::_integral_value;
};

typedef MaterialTensorAverageTempl<false> MaterialTensorAverage;
typedef MaterialTensorAverageTempl<true> ADMaterialTensorAverage;

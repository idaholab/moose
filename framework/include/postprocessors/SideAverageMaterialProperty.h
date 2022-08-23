//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralMaterialProperty.h"

/**
 * Computes the average of a material property over a side set.
 */
template <bool is_ad>
class SideAverageMaterialPropertyTempl : public SideIntegralMaterialPropertyTempl<is_ad>
{
public:
  static InputParameters validParams();

  SideAverageMaterialPropertyTempl(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  using SideIntegralMaterialPropertyTempl<is_ad>::_integral_value;
  /// Side set area
  Real _area;
};

typedef SideAverageMaterialPropertyTempl<false> SideAverageMaterialProperty;
typedef SideAverageMaterialPropertyTempl<true> ADSideAverageMaterialProperty;

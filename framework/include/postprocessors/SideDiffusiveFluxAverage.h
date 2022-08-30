//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideDiffusiveFluxIntegral.h"

template <bool>
class SideDiffusiveFluxAverageTempl;
typedef SideDiffusiveFluxAverageTempl<false> SideDiffusiveFluxAverage;
typedef SideDiffusiveFluxAverageTempl<true> ADSideDiffusiveFluxAverage;

template <bool is_ad>
class SideDiffusiveFluxAverageTempl : public SideDiffusiveFluxIntegralTempl<is_ad, Real>
{
public:
  static InputParameters validParams();

  SideDiffusiveFluxAverageTempl(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _volume;
  using SideDiffusiveFluxIntegralTempl<is_ad, Real>::_integral_value;
};

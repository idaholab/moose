//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceDiffusiveFluxIntegral.h"

template <bool>
class InterfaceDiffusiveFluxAverageTempl;
typedef InterfaceDiffusiveFluxAverageTempl<false> InterfaceDiffusiveFluxAverage;
typedef InterfaceDiffusiveFluxAverageTempl<true> ADInterfaceDiffusiveFluxAverage;

template <bool is_ad>
class InterfaceDiffusiveFluxAverageTempl : public InterfaceDiffusiveFluxIntegralTempl<is_ad>
{
public:
  static InputParameters validParams();

  InterfaceDiffusiveFluxAverageTempl(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual Real getValue() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  Real _volume;
  using InterfaceDiffusiveFluxIntegralTempl<is_ad>::_integral_value;
};

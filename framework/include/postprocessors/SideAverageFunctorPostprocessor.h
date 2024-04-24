//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralFunctorPostprocessor.h"

/**
 * Computes the average of a functor over a side set.
 */
class SideAverageFunctorPostprocessor : public SideIntegralFunctorPostprocessorTempl<false>
{
public:
  static InputParameters validParams();

  SideAverageFunctorPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual Real getValue() const override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  using SideIntegralFunctorPostprocessorTempl<false>::_integral_value;
  /// Side set area
  Real _area;
};

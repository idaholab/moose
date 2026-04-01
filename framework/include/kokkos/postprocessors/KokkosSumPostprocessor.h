//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosGeneralPostprocessor.h"

/**
 * Computes a sum of postprocessor values
 */
class KokkosSumPostprocessor : public Moose::Kokkos::GeneralPostprocessor
{
public:
  static InputParameters validParams();
  KokkosSumPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void compute() override {}
  virtual void finalize() override;
  virtual PostprocessorValue getValue() const override { return _sum; }

protected:
  /// Postprocessors to add up
  std::vector<const PostprocessorValue *> _values;
  /// The sum value
  Real _sum;
};

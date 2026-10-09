//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KokkosAverageValue.h"

/**
 * Kokkos postprocessor computing the effective thermal conductivity from the average temperature
 * on a boundary
 */
class KokkosThermalConductivity : public KokkosSideAverageValue
{
public:
  static InputParameters validParams();

  KokkosThermalConductivity(const InputParameters & parameters);

  virtual void finalize() override;
  virtual Real getValue() const override;

private:
  /// Length between sides of the sample
  const Real _dx;
  /// Heat flux out of the cold boundary
  const PostprocessorValue & _flux;
  /// Temperature on the hot boundary
  const PostprocessorValue & _T_hot;
  /// Length scale of the solution
  const Real _length_scale;
  /// Initial value of the thermal conductivity
  const Real _k0;
  /// Whether the first time step has not yet been completed
  bool & _step_zero;
  /// Computed thermal conductivity
  Real _value;
};

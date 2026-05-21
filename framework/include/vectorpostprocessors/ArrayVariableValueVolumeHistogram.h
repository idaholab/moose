//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVectorPostprocessor.h"

/**
 * Compute histograms of volume fractions binned according to component values of an array variable.
 * This VectorPostprocessor lets you tabulate the volumes in the simulation domain
 * where a given array variable has certain values.
 */
class ArrayVariableValueVolumeHistogram : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  ArrayVariableValueVolumeHistogram(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// number of histogram bins
  const unsigned int _nbins;

  /// minimum variable value
  const Real _min_value;

  /// maximum variable value
  const Real _max_value;

  /// bin width
  const Real _deltaV;

  /// coupled variable that is being binned
  const ArrayVariableValue & _value;

  /// coupled array variable
  const ArrayMooseVariable & _var;

  /// current quadrature point - used in computeVolume()
  unsigned int _qp;

  /// value mid point of the bin
  VectorPostprocessorValue & _bin_center;

  /// aggregated volumes of all components for the given bin
  std::vector<VectorPostprocessorValue *> _volumes;
};

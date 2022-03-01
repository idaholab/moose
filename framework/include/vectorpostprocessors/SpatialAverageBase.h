//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementVectorPostprocessor.h"

/**
 * Base clase for computing spatial average of a variable over simple spatial regions
 * of the computation domain
 */
class SpatialAverageBase : public ElementVectorPostprocessor
{
public:
  static InputParameters validParams();

  SpatialAverageBase(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// compute the distance of the current quadarature point for binning
  virtual Real computeDistance() = 0;

  /// number of histogram bins
  const unsigned int _nbins;

  /// maximum variable value
  const Real _radius;

  /// origin of sphere [or other body]
  const Point _origin;

  /// bin width
  const Real _deltaR;

  /// number of coupled variables
  const unsigned int _nvals;

  /// coupled variable that is being binned
  const std::vector<const VariableValue *> _values;

  /// current quadrature point - used in computeVolume()
  unsigned int _qp;

  /// value to assign to empty bins
  const Real _empty_bin_value;

  /// value mid point of the bin
  VectorPostprocessorValue & _bin_center;

  /// sample count per bin
  std::vector<unsigned int> _counts;

  /// aggregated global average vectors
  std::vector<VectorPostprocessorValue *> _average;
};

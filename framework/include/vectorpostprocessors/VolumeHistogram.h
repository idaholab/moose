//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VOLUMEHISTOGRAM_H
#define VOLUMEHISTOGRAM_H

#include "ElementVectorPostprocessor.h"

class VolumeHistogram;

template <>
InputParameters validParams<VolumeHistogram>();

/**
 * Compute a histogram of volume fractions binned according to variable values.
 * This VectorPostprocessor lets you tabulate the volumes in teh simulation domain
 * where a given variable has certain values.
 */
class VolumeHistogram : public ElementVectorPostprocessor
{
public:
  VolumeHistogram(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// compute the volume contribution at the current quadrature point
  virtual Real computeVolume();

  /// number of histogram bins
  const unsigned int _nbins;

  /// minimum variable value
  const Real _min_value;

  /// maximum variable value
  const Real _max_value;

  /// bin width
  const Real _deltaV;

  /// coupled variable that is being binned
  const VariableValue & _value;

  /// current quadrature point - used in computeVolume()
  unsigned int _qp;

  /// value mid point of the bin
  VectorPostprocessorValue & _bin_center;

  /// aggregated volume for the given bin
  VectorPostprocessorValue & _volume;
};

#endif // VOLUMEHISTOGRAM_H

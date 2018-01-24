/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef SPHERICALAVERAGE_H
#define SPHERICALAVERAGE_H

#include "ElementVectorPostprocessor.h"

class SphericalAverage;

template <>
InputParameters validParams<SphericalAverage>();

/**
 * Compute a spherical average of a variableas a function of radius throughout the
 * simulation domain.
 */
class SphericalAverage : public ElementVectorPostprocessor
{
public:
  SphericalAverage(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void finalize() override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// compute the distance of the current quadarature point for binning
  virtual Real computeDistance();

  /// number of histogram bins
  const unsigned int _nbins;

  /// maximum variable value
  const Real _radius;

  /// bin width
  const Real _deltaR;

  /// number of coupled variables
  const unsigned int _nvals;

  /// coupled variable that is being binned
  std::vector<const VariableValue *> _values;

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

#endif // SPHERICALAVERAGE_H

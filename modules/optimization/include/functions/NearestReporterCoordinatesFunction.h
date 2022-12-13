//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "OptimizationFunction.h"
#include "ReporterInterface.h"
/**
 * Function based on the nearest point to coordinates and values defined by a
 * vector of values, interpolates linearly in time with transient data.
 */
class NearestReporterCoordinatesFunction : public OptimizationFunction, public ReporterInterface
{
public:
  static InputParameters validParams();

  NearestReporterCoordinatesFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual RealGradient gradient(Real t, const Point & p) const override;
  virtual Real timeDerivative(Real t, const Point & p) const override;
  virtual std::vector<Real> parameterGradient(Real t, const Point & p) const override;

protected:
  /**
   * Builds _coord_mapping object with coordinates from input vectors
   */
  void buildCoordinateMapping() const;

  /**
   * With an input time and point, gets the closest point and two closest times in
   * _coord_mapping. See ::value on how the return value is used.
   *
   * @param t Requested time to interpolate
   * @param p Requrested point to find the closest one in the supplied data
   * @return Two pairs indicating the two closest time points and the corresponding index in
   * _values. The pairs will be equal if @param t is less than the smallest value or greater than
   * the largest value in _coord_mapping for the found nearest point.
   */
  std::array<std::pair<Real, std::size_t>, 2> findNearestPoint(Real t, const Point & p) const;

  /// x-coordinates from reporter
  const std::vector<Real> & _coordx;
  /// y-coordinates from reporter
  const std::vector<Real> & _coordy;
  /// z-coordinates from reporter
  const std::vector<Real> & _coordz;
  /// time-coordinates from reporter
  const std::vector<Real> & _coordt;
  /// values from reporter
  const std::vector<Real> & _values;

  /// Number of values from coordinate vectors
  mutable std::size_t _nval;
  /// Data structure for all current data
  mutable std::map<Point, std::vector<std::pair<Real, std::size_t>>> _coord_mapping;

private:
  const std::vector<Real> _empty_vec = {};
};

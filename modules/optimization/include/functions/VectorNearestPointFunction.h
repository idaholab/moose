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

class VectorNearestPointFunction : public OptimizationFunction, public ReporterInterface
{
public:
  static InputParameters validParams();

  VectorNearestPointFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual RealGradient gradient(Real t, const Point & p) const override;
  virtual Real timeDerivative(Real t, const Point & p) const override;
  virtual std::vector<Real> parameterGradient(Real t, const Point & p) const override;

protected:
  /**
   * Builds _coord_mapping object with coordinates from inputted vectors
   */
  void buildCoordinateMapping() const;

  /**
   * With an inputted time and point, gets the closest point and two closest times in
   * _coord_mapping. See ::value on how the return value is used.
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

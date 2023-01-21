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

#include "ParameterMesh.h"

/**
 *
 */
class ParameterMeshFunction : public OptimizationFunction, public ReporterInterface
{
public:
  static InputParameters validParams();

  ParameterMeshFunction(const InputParameters & parameters);

  using Function::value;
  virtual Real value(Real t, const Point & p) const override;
  virtual RealGradient gradient(Real t, const Point & p) const override;
  virtual Real timeDerivative(Real t, const Point & p) const override;
  virtual std::vector<Real> parameterGradient(Real t, const Point & p) const override;

protected:
  /**
   * This function is used to compute the weights for time interpolation
   * @see value for an example of how to use the return data
   *
   * @param t Time to interpolate to
   * @param derivate Set to true to compute time derivative
   * @return std::array<std::pair<Real, std::size_t>, 2>  Usage:
   *    const auto ti = interpolateTime(t);
   *    return v[ti[0].first] * ti[0].second + v[ti[1].first] * ti.[1].second;
   */
  std::array<std::pair<std::size_t, Real>, 2> interpolateTime(Real t,
                                                              bool derivative = false) const;

  /// Used to make sure DoFs in '_parameter_mesh' matches number of values in '_values'
  void checkSize() const;

  /// Parameter mesh
  const ParameterMesh _parameter_mesh;
  /// values from reporter
  const std::vector<Real> & _values;
  /// Time coordinates from reporter
  const std::vector<Real> & _coordt;

private:
  const std::vector<Real> _empty_vec = {};
};

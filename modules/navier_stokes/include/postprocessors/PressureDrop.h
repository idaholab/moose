//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideIntegralPostprocessor.h"

/**
 * This postprocessor computes the pressure drop between an upstream and a downstream boundary.
 * In case multiple boundaries are specified, or the pressure profile is not constant along the
 * boundaries, a vector weighting factor may be used.
 */
class PressureDrop : public SideIntegralPostprocessor
{
public:
  static InputParameters validParams();

  PressureDrop(const InputParameters & parameters);

  virtual Real computeQpIntegral() override { mooseError("Not implemented"); };

  virtual void initialize() override;
  virtual void meshChanged() override;
  virtual void execute() override;
  virtual void threadJoin(const UserObject & y) override;
  virtual void finalize() override;
  virtual Real getValue() override;

protected:
  /// Computes the contribution on a face to the weighted pressure integral
  Real computeFaceInfoWeightedPressureIntegral(const FaceInfo * fi) const;
  /// Computes the contribution on a face to the integral of the weight
  Real computeFaceInfoWeightIntegral(const FaceInfo * fi) const;

  /// Computes the contribution on a Qp to the weighted pressure integral
  Real computeQpWeightedPressureIntegral() const;
  /// Computes the contribution on a Qp to the integral of the weight
  Real computeQpWeightIntegral() const;

  /// The pressure functor
  const Moose::Functor<Real> & _pressure;
  /// A weighting functor if the pressure profile is not uniform
  const Moose::Functor<RealVectorValue> * const _weighting_functor;
  /// The interpolation method to use for the weighting functor quantity
  Moose::FV::InterpMethod _weight_interp_method;
  /// Vector of the ids of the upstream boundaries
  std::vector<BoundaryID> _upstream_boundaries;
  /// Vector of the ids of the downstream boundaries
  std::vector<BoundaryID> _downstream_boundaries;
  /// The weighted integral of the upstream pressure
  Real _weighted_pressure_upstream;
  /// The weighted integral of the downstream pressure
  Real _weighted_pressure_downstream;
  /// The integral of the weights on the upstream boundary, for normalization
  Real _weight_upstream;
  /// The integral of the weights on the downstream boundary, for normalization
  Real _weight_downstream;
};

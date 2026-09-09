//* This file is part of the MOOSE framework
//* https://www.mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SidePostprocessor.h"

/**
 * Computes the average contact angle that the phase field interface makes with a boundary.
 *
 * The angle is obtained from a weighted average of cos(theta) = grad(pf).n/|grad(pf)| over the
 * boundary. The weight |grad(pf)| * (1 - pf^2) removes the pointwise division and vanishes in the
 * bulk phases, so the average reduces to a ratio of two boundary integrals.
 */
class ObtainAvgContactAngle : public SidePostprocessor
{
public:
  static InputParameters validParams();

  ObtainAvgContactAngle(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void execute() override;
  using Postprocessor::getValue;
  virtual Real getValue() const override;
  virtual void threadJoin(const UserObject & y) override;

protected:
  /// Value of the phase field variable, used to localize the average to the interface
  const VariableValue & _pf;

  /// Gradient of the phase field variable
  const VariableGradient & _grad_pf;

  /// Average contact angle, in degrees
  Real _contact_angle;

  /// Boundary integral of (1 - pf^2) * grad(pf).n, the numerator of the averaged cos(theta)
  Real _cos_theta_val;

  /// Boundary integral of (1 - pf^2) * |grad(pf)|, the normalization of _cos_theta_val
  Real _total_weight;
};

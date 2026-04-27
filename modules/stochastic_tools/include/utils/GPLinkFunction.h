//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include <limits>
#include <memory>
#include <string>

namespace StochasticTools
{

/**
 * Enumeration of supported output link function types.
 * Stored as an integer in model data for serialization.
 */
enum class GPLinkFunctionType : int
{
  Identity = 0, ///< No transformation (default)
  Log = 1,      ///< g(y) = log(y - lb), enforces y > lb
  Logit = 2     ///< g(y) = log((y-lb)/(ub-y)), enforces lb < y < ub
};

/**
 * Utility class for GP output link functions. A link function g maps
 * physical-space outputs y to the GP training space z = g(y) and back via g^{-1}.
 * This allows GP regression to operate in a transformed space that naturally
 * satisfies inequality constraints such as positivity or bounded outputs.
 *
 * The transformation changes the negative log-marginal likelihood (NLML) by the
 * Jacobian correction: NLML_total = NLML_z - sum_i log|g'(y_i)|.
 * Since this Jacobian does not depend on the kernel hyperparameters, it does not
 * alter the hyperparameter gradient computation.
 *
 * Prediction uncertainty propagation uses the delta method:
 *   sigma_y ≈ |g^{-1}'(mu_z)| * sigma_z
 */
class GPLinkFunction
{
public:
  virtual ~GPLinkFunction() = default;

  /// Forward: physical space y -> GP training space z = g(y)
  virtual Real forward(Real y) const = 0;

  /// Inverse: GP training space z -> physical space y = g^{-1}(z)
  virtual Real inverse(Real z) const = 0;

  /// Derivative of the inverse: d/dz [g^{-1}(z)], used in delta-method uncertainty propagation
  virtual Real inverseDeriv(Real z) const = 0;

  /// log |g'(y)| = log |d/dy g(y)|, used as the Jacobian correction to NLML reporting
  virtual Real logJacobian(Real y) const = 0;

  /// Lowest physical-space value that is valid for this link function
  virtual Real lowerBound() const = 0;

  /// Highest physical-space value that is valid for this link function
  virtual Real upperBound() const = 0;

  /// Type tag for serialization
  virtual GPLinkFunctionType type() const = 0;

  /// Factory: create a link function from type tag and optional bounds
  static std::unique_ptr<GPLinkFunction>
  build(GPLinkFunctionType type, Real lb = 0.0, Real ub = 1.0);

  /// Factory: create a link function from a string name ("identity", "log", "logit")
  static std::unique_ptr<GPLinkFunction>
  buildFromString(const std::string & type_name, Real lb = 0.0, Real ub = 1.0);
};

/// Identity link: g(y) = y. No constraint is enforced.
class IdentityLinkFunction : public GPLinkFunction
{
public:
  Real forward(Real y) const override { return y; }
  Real inverse(Real z) const override { return z; }
  Real inverseDeriv(Real /*z*/) const override { return 1.0; }
  Real logJacobian(Real /*y*/) const override { return 0.0; }
  Real lowerBound() const override { return -std::numeric_limits<Real>::infinity(); }
  Real upperBound() const override { return std::numeric_limits<Real>::infinity(); }
  GPLinkFunctionType type() const override { return GPLinkFunctionType::Identity; }
};

/**
 * Log link: g(y) = log(y - lb), g^{-1}(z) = exp(z) + lb.
 * Enforces y > lb (typically lb = 0 for positivity).
 * Uncertainty propagation: sigma_y ≈ exp(mu_z) * sigma_z.
 */
class LogLinkFunction : public GPLinkFunction
{
public:
  LogLinkFunction(Real lb = 0.0) : _lb(lb) {}

  Real forward(Real y) const override;
  Real inverse(Real z) const override;
  Real inverseDeriv(Real z) const override;
  Real logJacobian(Real y) const override;
  Real lowerBound() const override { return _lb; }
  Real upperBound() const override { return std::numeric_limits<Real>::infinity(); }
  GPLinkFunctionType type() const override { return GPLinkFunctionType::Log; }
  Real lb() const { return _lb; }

private:
  Real _lb;
};

/**
 * Logit link: g(y) = log((y-lb)/(ub-y)), g^{-1}(z) = lb + (ub-lb)/(1+exp(-z)).
 * Enforces lb < y < ub.
 * Uncertainty propagation: sigma_y ≈ (ub-lb)*sigmoid(mu_z)*(1-sigmoid(mu_z)) * sigma_z.
 */
class LogitLinkFunction : public GPLinkFunction
{
public:
  LogitLinkFunction(Real lb = 0.0, Real ub = 1.0) : _lb(lb), _ub(ub) {}

  Real forward(Real y) const override;
  Real inverse(Real z) const override;
  Real inverseDeriv(Real z) const override;
  Real logJacobian(Real y) const override;
  Real lowerBound() const override { return _lb; }
  Real upperBound() const override { return _ub; }
  GPLinkFunctionType type() const override { return GPLinkFunctionType::Logit; }
  Real lb() const { return _lb; }
  Real ub() const { return _ub; }

private:
  Real _lb, _ub;
};

} // namespace StochasticTools

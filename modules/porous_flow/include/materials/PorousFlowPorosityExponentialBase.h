//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PorousFlowPorosityBase.h"

/**
 * Base class Material designed to provide the porosity.
 * In this class
 * porosity = a + (b - a) * exp(decay)
 * where
 * a = atNegInfinityQp()
 * b = atZeroQp()
 * decay = decayQp()
 * Since this expression can become negative for decay > 0,
 * if ensure_positive = true then for decay > 0 the following
 * expression is used instead:
 * porosity = a + (b - a) * exp(c * (1 - Exp(- decay / c)))
 * where c = log(a/(b-a))
 * This latter expression is C1 continuous at decay=0 with
 * the former expression.  It is monotonically decreasing
 * with "decay" and is positive.
 */
class PorousFlowPorosityExponentialBase : public PorousFlowPorosityBase
{
public:
  static InputParameters validParams();

  PorousFlowPorosityExponentialBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Returns "a" at the quadpoint (porosity = a + (b - a) * exp(decay))
  virtual Real atNegInfinityQp() const = 0;

  /// d(a)/d(PorousFlow variable pvar)
  virtual Real datNegInfinityQp(unsigned pvar) const = 0;

  /// Returns "b" at the quadpoint (porosity = a + (b - a) * exp(decay))
  virtual Real atZeroQp() const = 0;

  /// d(a)/d(PorousFlow variable pvar)
  virtual Real datZeroQp(unsigned pvar) const = 0;

  /// Returns "decay" at the quadpoint (porosity = a + (b - a) * exp(decay))
  virtual Real decayQp() const = 0;

  /// d(decay)/d(PorousFlow variable pvar)
  virtual Real ddecayQp_dvar(unsigned pvar) const = 0;

  /// d(decay)/d(grad(PorousFlow variable pvar))
  virtual RealGradient ddecayQp_dgradvar(unsigned pvar) const = 0;

  /// When calculating nodal porosity, use the strain at the nearest quadpoint to the node
  const bool _strain_at_nearest_qp;

  /**
   * for decayQp() > 0, porosity can be negative when using
   * porosity = a + (b - a) * exp(decay).
   * This expression is modified if ensure_positive = true to read
   * porosity = a + (b - a) * exp(c * (1 - Exp(- decay / c)))
   * where c = log(a/(b-a))
   */
  const bool _ensure_positive;
};

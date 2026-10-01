//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
template <bool is_ad>
class PorousFlowPorosityExponentialBaseTempl : public PorousFlowPorosityBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  PorousFlowPorosityExponentialBaseTempl(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Returns "a" at the quadpoint (porosity = a + (b - a) * exp(decay))
  virtual GenericReal<is_ad> atNegInfinityQp() const = 0;

  /// d(a)/d(PorousFlow variable pvar)
  virtual Real datNegInfinityQp(unsigned pvar) const = 0;

  /// Returns "b" at the quadpoint (porosity = a + (b - a) * exp(decay))
  virtual GenericReal<is_ad> atZeroQp() const = 0;

  /// d(b)/d(PorousFlow variable pvar)
  virtual Real datZeroQp(unsigned pvar) const = 0;

  /// Returns "decay" at the quadpoint (porosity = a + (b - a) * exp(decay))
  virtual GenericReal<is_ad> decayQp() const = 0;

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

  /**
   * Minimum allowed porosity.  The ensure_positive transform above only acts for
   * decay > 0, so chemistry-driven porosity (which enters through "a" and "b", not
   * "decay") is otherwise unbounded below and can go negative once pore space is
   * filled by precipitated mineral.  If the computed porosity is less than this
   * value it is set to this value instead.  Defaults to no floor.
   */
  const Real _porosity_min;

  /**
   * If the porosity_min floor is active the porosity derivatives are set to
   * _zero_modifier times their unfloored values (rather than exactly zero) to
   * hint to the Newton process that porosity is not strictly constant, which aids
   * convergence.
   */
  const Real _zero_modifier;

  usingPorousFlowPorosityBaseMembers;
};

#define usingPorousFlowPorosityExponentialBaseMembers                                              \
  usingPorousFlowPorosityBaseMembers;                                                              \
  using Coupleable::coupledComponents;                                                             \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_dictator;                                  \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_nodal_material;                            \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::nodalOrQpValue;                             \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::nearestQP;                                  \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_current_elem;                              \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_current_side;                              \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_bnd;                                       \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_qrule;                                     \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_q_point;                                   \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_constant_option;                           \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_t_step;                                    \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_dt;                                        \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_app;                                       \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_strain_at_nearest_qp;                      \
  using PorousFlowPorosityExponentialBaseTempl<is_ad>::_ensure_positive

typedef PorousFlowPorosityExponentialBaseTempl<false> PorousFlowPorosityExponentialBase;
typedef PorousFlowPorosityExponentialBaseTempl<true> ADPorousFlowPorosityExponentialBase;

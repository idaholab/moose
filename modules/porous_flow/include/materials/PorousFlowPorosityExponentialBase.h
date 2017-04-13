/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef POROUSFLOWPOROSITYEXPONENTIALBASE_H
#define POROUSFLOWPOROSITYEXPONENTIALBASE_H

#include "PorousFlowPorosityBase.h"

// Forward Declarations
class PorousFlowPorosityExponentialBase;

template <>
InputParameters validParams<PorousFlowPorosityExponentialBase>();

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
  PorousFlowPorosityExponentialBase(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;
  virtual void computeQpProperties() override;

  /// Returns "a" at the quadpoint (porosity = a + (b - a) * exp(decay))
  virtual Real atNegInfinityQp() const;

  /// Returns "b" at the quadpoint (porosity = a + (b - a) * exp(decay))
  virtual Real atZeroQp() const;

  /// Returns "decay" at the quadpoint (porosity = a + (b - a) * exp(decay))
  virtual Real decayQp() const;

  /// d(decay)/d(porous-flow variable pvar)
  virtual Real ddecayQp_dvar(unsigned pvar) const;

  /// d(decay)/d(grad(porous-flow variable pvar))
  virtual RealGradient ddecayQp_dgradvar(unsigned pvar) const;

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

#endif // POROUSFLOWPOROSITYEXPONENTIALBASE_H

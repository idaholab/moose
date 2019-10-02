#pragma once

#include "SmoothTransitionInterface.h"

class WeightedTransitionInterface;

template <>
InputParameters validParams<WeightedTransitionInterface>();

/**
 * Interface class for weighted transition between two functions of one variable
 */
class WeightedTransitionInterface : public SmoothTransitionInterface
{
public:
  WeightedTransitionInterface(const MooseObject * moose_object);

protected:
  virtual Real
  computeTransitionValue(const Real & x, const Real & f1, const Real & f2) const override;

  Real computeTransitionValueDerivative(const Real & x,
                                        const Real & f1,
                                        const Real & f2,
                                        const Real & df1dx,
                                        const Real & df2dx) const;

  Real computeWeight(const Real & x) const;
};

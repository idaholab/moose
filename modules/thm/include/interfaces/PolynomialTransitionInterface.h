#pragma once

#include "SmoothTransitionInterface.h"

class PolynomialTransitionInterface;

template <>
InputParameters validParams<PolynomialTransitionInterface>();

/**
 * Interface class for cubic polynomial transition between two functions of one variable
 */
class PolynomialTransitionInterface : public SmoothTransitionInterface
{
public:
  PolynomialTransitionInterface(const MooseObject * moose_object);

protected:
  virtual Real
  computeTransitionValue(const Real & x, const Real & f1, const Real & f2) const override;

  Real
  computeTransitionValueDerivative(const Real & x, const Real & df1dx, const Real & df2dx) const;

  /**
   * Initializes the polynomial coefficients
   *
   * @param[in] f1_end_value   Value of left function at left transition end point
   * @param[in] f2_end_value   Value of right function at right transition end point
   * @param[in] df1dx_end_value   Value of left function derivative at left transition end point
   * @param[in] df2dx_end_value   Value of right function derivative at right transition end point
   */
  void initializeTransitionData(const Real & f1_end_value,
                                const Real & f2_end_value,
                                const Real & df1dx_end_value,
                                const Real & df2dx_end_value);

  // Polynomial coefficients
  Real _A;
  Real _B;
  Real _C;
  Real _D;

  /// Flag that transition data has been initialized
  bool _initialized_transition_data;
};

#pragma once

#include "ADSmoothTransition.h"

/**
 * Weighted transition between two functions of one variable
 */
class ADWeightedTransition : public ADSmoothTransition
{
public:
  /**
   * Constructor.
   *
   * @param[in] x_center   Center point of transition
   * @param[in] transition_width   Width of transition
   */
  ADWeightedTransition(const ADReal & x_center, const ADReal & transition_width);

  virtual ADReal value(const ADReal & x, const ADReal & f1, const ADReal & f2) const override;

  /**
   * Computes the weight of the first function
   *
   * @param[in] x   Point at which to evaluate weight
   */
  ADReal weight(const ADReal & x) const;
};

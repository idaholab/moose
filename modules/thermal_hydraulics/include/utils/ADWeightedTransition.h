//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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

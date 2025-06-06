//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ChangeOverTimePostprocessor.h"

/**
 * Computes the change in a post-processor value, or the magnitude of its
 * relative change, over a time step or over the entire transient.
 */
class ChangeOverTimestepPostprocessor : public ChangeOverTimePostprocessor
{
public:
  static InputParameters validParams();

  ChangeOverTimestepPostprocessor(const InputParameters & parameters);
};

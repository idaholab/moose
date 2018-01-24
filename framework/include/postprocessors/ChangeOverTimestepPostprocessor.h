//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CHANGEOVERTIMESTEPPOSTPROCESSOR_H
#define CHANGEOVERTIMESTEPPOSTPROCESSOR_H

#include "ChangeOverTimePostprocessor.h"

class ChangeOverTimestepPostprocessor;

template <>
InputParameters validParams<ChangeOverTimestepPostprocessor>();

/**
 * Computes the change in a post-processor value, or the magnitude of its
 * relative change, over a time step or over the entire transient.
 */
class ChangeOverTimestepPostprocessor : public ChangeOverTimePostprocessor
{
public:
  ChangeOverTimestepPostprocessor(const InputParameters & parameters);
};

#endif /* CHANGEOVERTIMESTEPPOSTPROCESSOR_H */

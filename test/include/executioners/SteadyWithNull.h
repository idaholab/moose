//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef STEADYWITHNULL_H
#define STEADYWITHNULL_H

#include "Steady.h"

class SteadyWithNull;

template <>
InputParameters validParams<SteadyWithNull>();

/**
 * Steady excecutioner setting nullspace
 */
class SteadyWithNull : public Steady
{
public:
  SteadyWithNull(const InputParameters & parameters);

  virtual void init() override;
};

#endif /* SteadyWithNull_H */

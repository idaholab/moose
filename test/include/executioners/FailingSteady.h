//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FAILINGSTEADY_H
#define FAILINGSTEADY_H

#include "Steady.h"

class FailingSteady;

template <>
InputParameters validParams<FailingSteady>();

/**
 * Steady derived class that will fail a prescribed timestep for testing
 * timestepping algorithms
 */
class FailingSteady : public Steady
{
public:
  FailingSteady(const InputParameters & params);
  virtual bool augmentedFEProblemSolveFail() override;

protected:
  bool _failed;
  unsigned int _fail_step;
};

#endif /* FAILINGTRANSIENT_H */

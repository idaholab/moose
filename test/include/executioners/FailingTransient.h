//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef FAILINGTRANSIENT_H
#define FAILINGTRANSIENT_H

#include "Transient.h"

class FailingTransient;

template <>
InputParameters validParams<FailingTransient>();

/**
 * Transient derived class that will fail a prescribed timestep for testing
 * timestepping algorithms
 */
class FailingTransient : public Transient
{
public:
  FailingTransient(const InputParameters & params);
  virtual bool augmentedFEProblemSolveFail() override;

protected:
  bool _failed;
  unsigned int _fail_step;
};

#endif /* FAILINGTRANSIENT_H */

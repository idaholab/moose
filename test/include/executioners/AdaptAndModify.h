//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Transient.h"

class AdaptAndModify : public Transient
{
public:
  static InputParameters validParams();

  AdaptAndModify(const InputParameters & parameters);

  virtual void incrementStepOrReject();

  virtual void endStep(Real input_time = -1.0);

protected:
  unsigned int _adapt_cycles;
};

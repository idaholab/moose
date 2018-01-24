//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADAPTANDMODIFY_H
#define ADAPTANDMODIFY_H

#include "Transient.h"

// Forward Declarations
class AdaptAndModify;

template <>
InputParameters validParams<AdaptAndModify>();

class AdaptAndModify : public Transient
{
public:
  AdaptAndModify(const InputParameters & parameters);

  virtual void incrementStepOrReject();

  virtual void endStep(Real input_time = -1.0);

protected:
  unsigned int _adapt_cycles;
};

#endif // ADAPTANDMODIFY_H

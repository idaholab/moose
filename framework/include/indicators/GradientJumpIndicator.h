//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GRADIENTJUMPINDICATOR_H
#define GRADIENTJUMPINDICATOR_H

#include "InternalSideIndicator.h"

class GradientJumpIndicator;

template <>
InputParameters validParams<GradientJumpIndicator>();

class GradientJumpIndicator : public InternalSideIndicator
{
public:
  GradientJumpIndicator(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
};

#endif /* GRADIENTJUMPINDICATOR_H */

//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MTICSUM_H
#define MTICSUM_H

#include "InitialCondition.h"

class MTICSum;

template <>
InputParameters validParams<MTICSum>();

/**
 *
 */
class MTICSum : public InitialCondition
{
public:
  MTICSum(const InputParameters & parameters);
  virtual ~MTICSum();

  virtual Real value(const Point & /*p*/);

protected:
  const VariableValue & _var1;
  const VariableValue & _var2;
};

#endif /* MTICSUM_H */

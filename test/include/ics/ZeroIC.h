//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ZEROIC_H
#define ZEROIC_H

#include "InitialCondition.h"

class ZeroIC;

template <>
InputParameters validParams<ZeroIC>();

/**
 * Initial condition that accesses _zero
 */
class ZeroIC : public InitialCondition
{
public:
  ZeroIC(const InputParameters & parameters);

  virtual Real value(const Point & p);
};

#endif // ZEROIC_H

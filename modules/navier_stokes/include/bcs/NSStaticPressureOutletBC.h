//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NSSTATICPRESSUREOUTLETBC_H
#define NSSTATICPRESSUREOUTLETBC_H

#include "MooseObject.h"

class NSStaticPressureOutletBC;

template <>
InputParameters validParams<NSStaticPressureOutletBC>();

/**
 * This class facilitates adding specified static pressure outlet BCs for
 * the Euler equations.
 */
class NSStaticPressureOutletBC : public MooseObject
{
public:
  NSStaticPressureOutletBC(const InputParameters & parameters);
  virtual ~NSStaticPressureOutletBC();

protected:
};

#endif

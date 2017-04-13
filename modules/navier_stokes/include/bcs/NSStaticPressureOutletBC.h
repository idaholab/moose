/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
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

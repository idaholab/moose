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
#ifndef MTICMULT_H
#define MTICMULT_H

#include "InitialCondition.h"

class MTICMult;

template <>
InputParameters validParams<MTICMult>();

/**
 *
 */
class MTICMult : public InitialCondition
{
public:
  MTICMult(const InputParameters & parameters);
  virtual ~MTICMult();

  virtual Real value(const Point & /*p*/);

protected:
  const VariableValue & _var1;
  Real _factor;
};

#endif /* MTICMULT_H */

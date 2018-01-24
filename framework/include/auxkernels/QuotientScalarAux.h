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

#ifndef QUOTIENTSCALARAUX_H
#define QUOTIENTSCALARAUX_H

#include "AuxScalarKernel.h"

class QuotientScalarAux;

template <>
InputParameters validParams<QuotientScalarAux>();

/**
 * This auxiliary kernel computes its value by dividing "numerator" by
 * "denominator.  For efficiency, it doesn't check the denominator for
 * zero before dividing.
 */
class QuotientScalarAux : public AuxScalarKernel
{
public:
  QuotientScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  VariableValue & _a;
  VariableValue & _b;
};

#endif /* QUOTIENTSCALARAUX_H */

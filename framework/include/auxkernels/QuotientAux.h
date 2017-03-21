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

#ifndef QUOTIENTAUX_H
#define QUOTIENTAUX_H

#include "AuxKernel.h"

// Forward Declarations
class QuotientAux;

template <>
InputParameters validParams<QuotientAux>();

/**
 * This auxiliary kernel computes its value by dividing "numerator" by
 * "denominator.  For efficiency, it doesn't check the denominator for
 * zero before dividing.  Perhaps a derived class, CheckedQuotientAux,
 * could be added for people who want this feature.
 */
class QuotientAux : public AuxKernel
{
public:
  QuotientAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  const VariableValue & _numerator;
  const VariableValue & _denominator;
};

#endif // QUOTIENTAUX_H

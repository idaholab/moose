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

#ifndef ELEMENTW1PERROR_H
#define ELEMENTW1PERROR_H

#include "ElementIntegralVariablePostprocessor.h"

class Function;

// Forward Declarations
class ElementW1pError;

template <>
InputParameters validParams<ElementW1pError>();

/**
 * This postprocessor computes the Sobolev norm W^{1,p} of the
 * difference between the computed solution and the passed in function.
 * If p==2, this is equivalent to the H1-norm, but p can be any real
 * number >= 1.  There are two possible definitions of this norm:
 *
 * 1.) ||u-f||_{W^{1,p}} \equiv (\int |u-f|^p dx + sum_{i=1}^3 \int |du/dx_i - df/dx_i|^p dx)^{1/p}
 * 2.) ||u-f||_{W^{1,p}} \equiv (\int |u-f|^p dx)^{1/p} + sum_{i=1}^3 (\int |du/dx_i - df/dx_i|^p
 * dx)^{1/p}
 *
 * which are equivalent in the "equivalence of norms" sense.  (The
 * difference is that the "1/p" exponent is on the outside of the sum
 * in case 1, while it is on every term in case 2.  We use definition
 * 1 here for consistency with the original ElementH1Error class.
 */
class ElementW1pError : public ElementIntegralVariablePostprocessor
{
public:
  ElementW1pError(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;

  // The exponent used in the norm
  Real _p;
  Function & _func;
};

#endif // ELEMENTW1PERROR_H

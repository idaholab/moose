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

#ifndef ELEMENTL2DIFF_H
#define ELEMENTL2DIFF_H

#include "ElementIntegralVariablePostprocessor.h"

// Forward Declarations
class ElementL2Diff;

template <>
InputParameters validParams<ElementL2Diff>();

class ElementL2Diff : public ElementIntegralVariablePostprocessor
{
public:
  ElementL2Diff(const InputParameters & parameters);

protected:
  /**
   * Get the L2 Error.
   */
  virtual Real getValue();

  virtual Real computeQpIntegral();

  const VariableValue & _u_old;
};

#endif // ELEMENTL2DIFF_H

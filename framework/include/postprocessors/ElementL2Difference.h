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

#ifndef ELEMENTL2DIFFERENCE_H
#define ELEMENTL2DIFFERENCE_H

#include "ElementIntegralVariablePostprocessor.h"

// Forward Declarations
class ElementL2Difference;

template <>
InputParameters validParams<ElementL2Difference>();

/**
 * Computes the L2-Norm difference between two solution fields.
 */
class ElementL2Difference : public ElementIntegralVariablePostprocessor
{
public:
  ElementL2Difference(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;

  /// The variable to compare to
  const VariableValue & _other_var;
};

#endif // ELEMENTL2DIFFERENCE_H

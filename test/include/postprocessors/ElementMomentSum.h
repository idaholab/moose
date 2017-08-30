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

#ifndef ELEMENTMOMENTSUM_H
#define ELEMENTMOMENTSUM_H

#include "ElementIntegralVariablePostprocessor.h"

// Forward Declarations
class ElementMomentSum;

template <>
InputParameters validParams<ElementMomentSum>();

class ElementMomentSum : public ElementIntegralVariablePostprocessor
{
public:
  ElementMomentSum(const InputParameters & parameters);

protected:
  virtual void execute() override;

  const DenseVector<Number> & _elemental_sln;
};

#endif // ELEMENTMOMENTSUM_H

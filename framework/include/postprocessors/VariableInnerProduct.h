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

#ifndef VARIABLEINNERPRODUCT_H
#define VARIABLEINNERPRODUCT_H

#include "ElementIntegralVariablePostprocessor.h"

// Forward Declarations
class VariableInnerProduct;

template <>
InputParameters validParams<VariableInnerProduct>();

class VariableInnerProduct : public ElementIntegralVariablePostprocessor
{
public:
  VariableInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Holds the values of second_variable at current quadrature points
  const VariableValue & _v;
};

#endif // VariableInnerProduct_H

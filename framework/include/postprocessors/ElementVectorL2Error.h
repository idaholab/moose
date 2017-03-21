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

#ifndef ELEMENTVECTORL2ERROR_H
#define ELEMENTVECTORL2ERROR_H

#include "ElementIntegralPostprocessor.h"

class Function;

// Forward Declarations
class ElementVectorL2Error;

template <>
InputParameters validParams<ElementVectorL2Error>();

class ElementVectorL2Error : public ElementIntegralPostprocessor
{
public:
  ElementVectorL2Error(const InputParameters & parameters);

  virtual Real getValue() override;

protected:
  virtual Real computeQpIntegral() override;

  Function & _funcx;
  Function & _funcy;
  Function & _funcz;

  const VariableValue & _u; // FE solution in x
  const VariableValue & _v; // FE solution in y
  const VariableValue & _w; // FE solution in z
};

#endif // ELEMENTVECTORL2ERROR_H

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

#ifndef ElementSidePP_H
#define ElementSidePP_H

#include "ElementIntegralPostprocessor.h"

// Forward Declarations
class ElementSidePP;

template <>
InputParameters validParams<ElementSidePP>();

class ElementSidePP : public ElementIntegralPostprocessor
{
public:
  ElementSidePP(const InputParameters & parameters);

protected:
  virtual Real getValue();

  virtual Real computeQpIntegral();

  const PostprocessorValue & _sidepp;
};

#endif // ElementSidePP_H

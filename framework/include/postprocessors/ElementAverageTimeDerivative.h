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

#ifndef ELEMENTAVERAGETIMEDERIVATIVE_H
#define ELEMENTAVERAGETIMEDERIVATIVE_H

#include "ElementAverageValue.h"

//Forward Declarations
class ElementAverageTimeDerivative;

template<>
InputParameters validParams<ElementAverageTimeDerivative>();

/**
 * This postprocessor computes a volume integral of the time derivative of a given variable.
 */
class ElementAverageTimeDerivative : public ElementAverageValue
{
public:
  ElementAverageTimeDerivative(const std::string & name, InputParameters parameters);

  virtual Real computeQpIntegral();
};

#endif

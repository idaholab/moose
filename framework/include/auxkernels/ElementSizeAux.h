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
#ifndef ELEMENTSIZEAUX_H
#define ELEMENTSIZEAUX_H

// MOOSE includes
#include "AuxKernel.h"


// Forward Declarations
class ElementSizeAux;

template<>
InputParameters validParams<ElementSizeAux>();

/**
 * Computes the min or max of element size.
 */
class ElementSizeAux : public AuxKernel
{
public:
  ElementSizeAux(const InputParameters & parameters);

protected:

  /**
   * Returns the min/max of the current element.
   */
  virtual Real computeValue() override;

  /// The type of calculation to perform min or max
  const MooseEnum & _method;
};

#endif // ELEMENTSIZEAUX_H

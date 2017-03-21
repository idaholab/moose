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
#ifndef ELEMENTLENGTHAUX_H
#define ELEMENTLENGTHAUX_H

// MOOSE includes
#include "AuxKernel.h"

// Forward Declarations
class ElementLengthAux;

template <>
InputParameters validParams<ElementLengthAux>();

/**
 * Computes the min or max of element length.
 */
class ElementLengthAux : public AuxKernel
{
public:
  ElementLengthAux(const InputParameters & parameters);

protected:
  /**
   * Returns the min/max of the current element.
   */
  virtual Real computeValue() override;

  /// The type of calculation to perform min or max
  const bool _use_min;
};

#endif // ELEMENTLENGTHAUX_H

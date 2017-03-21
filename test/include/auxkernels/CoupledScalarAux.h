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

#ifndef COUPLEDSCALARAUX_H
#define COUPLEDSCALARAUX_H

#include "AuxKernel.h"

// Forward Declarations
class CoupledScalarAux;

template <>
InputParameters validParams<CoupledScalarAux>();

/**
 * Coupled auxiliary scalar value
 */
class CoupledScalarAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  CoupledScalarAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Coupled variable
  VariableValue & _coupled_val;

  /// The component of the scalar variable
  unsigned int _component;
};

#endif // COUPLEDSCALARAUX_H

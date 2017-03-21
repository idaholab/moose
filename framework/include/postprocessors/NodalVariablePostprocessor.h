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

#ifndef NODALVARIABLEPOSTPROCESSOR_H
#define NODALVARIABLEPOSTPROCESSOR_H

// MOOSE includes
#include "NodalPostprocessor.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class NodalVariablePostprocessor;

template <>
InputParameters validParams<NodalVariablePostprocessor>();

/**
 * This is a base class for other classes which compute post-processed
 * values based on nodal solution values of _u.
 */
class NodalVariablePostprocessor : public NodalPostprocessor, public MooseVariableInterface
{
public:
  NodalVariablePostprocessor(const InputParameters & parameters);

protected:
  /// Holds the solution at current quadrature points
  const VariableValue & _u;
};

#endif

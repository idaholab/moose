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

#ifndef NODALVARIABLEVECTORPOSTPROCESSOR_H
#define NODALVARIABLEVECTORPOSTPROCESSOR_H

#include "NodalVectorPostprocessor.h"

// Forward Declarations
class NodalVariableVectorPostprocessor;

template <>
InputParameters validParams<NodalVariableVectorPostprocessor>();

/**
 * Base class VectorPostprocessors operating on nodal variables.
 */
class NodalVariableVectorPostprocessor : public NodalVectorPostprocessor
{
public:
  NodalVariableVectorPostprocessor(const InputParameters & parameters);
};

#endif

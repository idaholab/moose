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

#ifndef VECTORPOSTPROCESSORAUX_H
#define VECTORPOSTPROCESSORAUX_H

#include "AuxKernel.h"

// Forward Declarations
class VectorPostprocessorAux;

template <>
InputParameters validParams<VectorPostprocessorAux>();

/**
 * Coupled auxiliary value
 */
class VectorPostprocessorAux : public AuxKernel
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  VectorPostprocessorAux(const InputParameters & parameters);

  virtual ~VectorPostprocessorAux() {}

protected:
  virtual Real computeValue();

  const VectorPostprocessorValue & _vpp;

  unsigned int _index;
};

#endif // VECTORPOSTPROCESSORAUX_H

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

#ifndef NODALVECTORPOSTPROCESSOR_H
#define NODALVECTORPOSTPROCESSOR_H

// MOOSE includes
#include "NodalUserObject.h"
#include "VectorPostprocessor.h"

// Forward Declarations
class NodalVectorPostprocessor;

template <>
InputParameters validParams<NodalVectorPostprocessor>();

class NodalVectorPostprocessor : public NodalUserObject, public VectorPostprocessor
{
public:
  NodalVectorPostprocessor(const InputParameters & parameters);

  /**
   * Finalize.  This is called _after_ execute() and _after_ threadJoin()!  This is probably where
   * you want to do MPI communication!
   */
  virtual void finalize() override {}
};

#endif

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

#ifndef GENERALVECTORPOSTPROCESSOR_H
#define GENERALVECTORPOSTPROCESSOR_H

// MOOSE includes
#include "VectorPostprocessor.h"
#include "GeneralUserObject.h"

// Forward Declarations
class GeneralVectorPostprocessor;

template <>
InputParameters validParams<GeneralVectorPostprocessor>();

/**
 * This class is here to combine the VectorPostprocessor interface and
 * the base class VectorPostprocessor object along with adding
 * MooseObject to the inheritance tree.
 */
class GeneralVectorPostprocessor : public GeneralUserObject, public VectorPostprocessor
{
public:
  GeneralVectorPostprocessor(const InputParameters & parameters);

  /**
   * Finalize.  This is called _after_ execute() and _after_
   * threadJoin()!  This is probably where you want to do MPI
   * communication!
   */
  virtual void finalize() override {}
};

#endif

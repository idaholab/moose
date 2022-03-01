//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "VectorPostprocessor.h"
#include "GeneralUserObject.h"

/**
 * This class is here to combine the VectorPostprocessor interface and
 * the base class VectorPostprocessor object along with adding
 * MooseObject to the inheritance tree.
 */
class GeneralVectorPostprocessor : public GeneralUserObject, public VectorPostprocessor
{
public:
  static InputParameters validParams();

  GeneralVectorPostprocessor(const InputParameters & parameters);

  /**
   * Finalize.  This is called _after_ execute() and _after_
   * threadJoin()!  This is probably where you want to do MPI
   * communication!
   */
  virtual void finalize() override {}
};

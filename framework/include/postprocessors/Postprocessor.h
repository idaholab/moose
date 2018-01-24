//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POSTPROCESSOR_H
#define POSTPROCESSOR_H

// MOOSE includes
#include "OutputInterface.h"

// libMesh
#include "libmesh/parallel.h"

// Forward declarations
class Postprocessor;

template <>
InputParameters validParams<Postprocessor>();

/**
 * Base class for all Postprocessors.  Defines a name and sets up the
 * virtual getValue() interface which must be overridden by derived
 * classes.
 */
class Postprocessor : public OutputInterface
{
public:
  Postprocessor(const InputParameters & parameters);

  /**
   * This will get called to actually grab the final value the postprocessor has calculated.
   */
  virtual PostprocessorValue getValue() = 0;

  /**
   * Returns the name of the Postprocessor.
   */
  std::string PPName() { return _pp_name; }

protected:
  std::string _pp_name;
};

#endif

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

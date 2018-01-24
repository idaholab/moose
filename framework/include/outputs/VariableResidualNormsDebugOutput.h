//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef VARIABLERESIDUALNORMSDEBUGOUTPUT_H
#define VARIABLERESIDUALNORMSDEBUGOUTPUT_H

// MOOSE includes
#include "PetscOutput.h"

#include "libmesh/system.h"

// Forward declerations
class VariableResidualNormsDebugOutput;

template <>
InputParameters validParams<VariableResidualNormsDebugOutput>();

/**
 * A class for producing various debug related outputs
 *
 * This class may be used from inside the [Outputs] block or via the [Debug] block (preferred)
 */
class VariableResidualNormsDebugOutput : public PetscOutput
{
public:
  /**
   * Class constructor
   * @param parameters Object input parameters
   */
  VariableResidualNormsDebugOutput(const InputParameters & parameters);

protected:
  /**
   * Perform the debugging output
   */
  virtual void output(const ExecFlagType & type) override;

  /// Reference to libMesh system
  System & _sys;
};

#endif // VARIABLERESIDUALNORMSDEBUGOUTPUT_H

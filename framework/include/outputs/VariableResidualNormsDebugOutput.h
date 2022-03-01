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
#include "PetscOutput.h"

#include "libmesh/system.h"

/**
 * A class for producing various debug related outputs
 *
 * This class may be used from inside the [Outputs] block or via the [Debug] block (preferred)
 */
class VariableResidualNormsDebugOutput : public PetscOutput
{
public:
  static InputParameters validParams();

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

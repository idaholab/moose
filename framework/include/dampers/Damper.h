//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose Includes
#include "MooseObject.h"
#include "SetupInterface.h"
#include "Restartable.h"
#include "MeshChangedInterface.h"

class SubProblem;
class SystemBase;

/**
 * Base class for deriving dampers
 */
class Damper : public MooseObject,
               public SetupInterface,
               public Restartable,
               public MeshChangedInterface
{
public:
  static InputParameters validParams();

  Damper(const InputParameters & parameters);

  /**
   * Check whether damping is below the user-specified minimum value,
   * and throw an exception if it is.
   * @param cur_damping The computed damping to be checked against that minimum
   */
  void checkMinDamping(const Real cur_damping) const;

protected:
  SubProblem & _subproblem;
  SystemBase & _sys;

  /// Minimum allowable value of damping
  const Real & _min_damping;
};

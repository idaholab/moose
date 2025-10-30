//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SCMClosureBase.h"
#include "SubChannel1PhaseProblem.h"

/**
 * Base class for friction closures used in SCM
 */
class SCMFrictionClosureBase : public SCMClosureBase
{
public:
  static InputParameters validParams();

  SCMFrictionClosureBase(const InputParameters & parameters);

  typedef SubChannel1PhaseProblem::FrictionStruct FrictionStruct;

  /// @brief Computes the friction factor for the local conditions
  /// @param friction_info geometrical information about the cell in the channel
  /// @return the friction factor
  virtual Real computeFrictionFactor(const FrictionStruct & friction_info) const = 0;
};

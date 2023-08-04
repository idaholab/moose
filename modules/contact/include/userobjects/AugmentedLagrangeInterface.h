//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class MooseObject;

/**
 * Interface class for user objects that support the augmented Lagrange formalism as
 * implemented in AugmentedLagrangianContactProblem.
 */
class AugmentedLagrangeInterface
{
public:
  AugmentedLagrangeInterface(const MooseObject * moose_object);

  virtual bool isAugmentedLagrangianConverged() = 0;
  virtual void augmentedLagrangianSetup() = 0;
  virtual void updateAugmentedLagrangianMultipliers() = 0;
};

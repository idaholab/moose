//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "PeriodicBCHelper.h"

class AddRayBCAction;

/**
 * Action that sets up the periodic boundary conditions
 * for a PeriodicRayBC.
 *
 * This Action is applied to _every_ RayBC, but it will
 * only perform its actions if the associated RayBC
 * is a PeriodicRayBC.
 */
class SetupPeriodicRayBCAction : public Action, public Moose::PeriodicBCHelper
{
public:
  SetupPeriodicRayBCAction(const InputParameters & params);

  static InputParameters validParams();

  void act() override final;

  /**
   * Method to be called from the AddRayBCAction that is associated
   * with the same RayBC this Action is associated with.
   *
   * Is used to set the necessary parameters for the PeriodicRayBC.
   */
  void setupPeriodicRayBC(InputParameters & params) const;

private:
  /**
   * Helper for getting the AddRayBCAction that is responsible for
   * building the RayBC this action is associated with
   */
  const AddRayBCAction & getAddRayBCAction() const;

  /// Whether or not we are acting on a PeriodicRayBC
  const bool _is_periodic_ray_bc;
};
